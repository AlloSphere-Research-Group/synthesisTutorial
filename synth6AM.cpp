/*    Gamma - Generic processing library
    See COPYRIGHT file for authors and license information

    Example:
    Description:
*/

#include <cstdio>               // for printing to stdout
#define GAMMA_H_INC_ALL         // define this to include all header files
#define GAMMA_H_NO_IO           // define this to avoid bringing AudioIO from Gamma

#include "Gamma/Gamma.h"
#include "Gamma/Types.h"

#include "al/core/app/al_App.hpp"
#include "al/core/graphics/al_Shapes.hpp"
#include "al/util/ui/al_Parameter.hpp"
#include "al/util/scene/al_PolySynth.hpp"
#include "al/util/scene/al_SynthSequencer.hpp"
#include "al/util/ui/al_ControlGUI.hpp"

using namespace gam;
using namespace al;

// tables for oscillator
gam::ArrayPow2<float>
tbSin(2048), tbSqr(2048), tbPls(2048), tbDin(2048);


class OscAM : public SynthVoice {
public:

  Osc<> mAM;
  gam::ADSR<> mAMEnv;
  gam::Osc<> mOsc;
  gam::ADSR<> mAmpEnv;
  EnvFollow<> mEnvFollow;
  Pan<> mPan;

  Mesh mMesh;

  void init( ) override {
    mAmpEnv.levels(0,1,1,0);
    mAMEnv.curve(0);

    // We have the mesh be a sphere
    addDisc(mMesh, 1.0, 30);

    createInternalTriggerParameter("frequency", 440, 10, 4000.0);
    createInternalTriggerParameter("amplitude", 0.5, 0.0, 1.0);
    createInternalTriggerParameter("attackTime", 0.1, 0.01, 3.0);
    createInternalTriggerParameter("releaseTime", 0.1, 0.1, 10.0);
    createInternalTriggerParameter("sustain", 0.75, 0.1, 1.0);
    createInternalTriggerParameter("am1", 0.75, 0.1, 1.0);
    createInternalTriggerParameter("am2", 0.75, 0.1, 1.0);
    createInternalTriggerParameter("amRise", 0.75, 0.1, 1.0);
    createInternalTriggerParameter("amRatio", 0.75, 0.1, 1.0);
    createInternalTriggerParameter("amFunc", 0.0, 0.0, 3.0);
    createInternalTriggerParameter("pan", 0.0, -1.0, 1.0);
  }

  virtual void onProcess(AudioIOData& io) override {
    mOsc.freq(getInternalParameterValue("frequency"));

    float amp = getInternalParameterValue("amplitude");
    float amRatio = getInternalParameterValue("amRatio");
    while(io()){

      mAM.freq(mOsc.freq()*amRatio);            // set AM freq according to ratio
      float amAmt = mAMEnv();                    // AM amount envelope

      float s1 = mOsc();                        // non-modulated signal
      s1 = s1*(1-amAmt) + (s1*mAM())*amAmt;    // mix modulated and non-modulated

      s1 *= mAmpEnv() *amp;

      float s2;
      mEnvFollow(s1);
      mPan(s1, s1,s2);
      io.out(0) += s1;
      io.out(1) += s2;
    }
    //if(mAmpEnv.done()) free();
    if(mAmpEnv.done() && (mEnvFollow.value() < 0.001)) free();
  }

  virtual void onTriggerOn() override {

    mAmpEnv.lengths()[0] = getInternalParameterValue("attackTime");
    mAmpEnv.lengths()[1] = 0.001;
    mAmpEnv.lengths()[2] = getInternalParameterValue("releaseTime");

    mAmpEnv.levels()[1]= getInternalParameterValue("sustain");
    mAmpEnv.levels()[2]= getInternalParameterValue("sustain");

    mAMEnv.levels(getInternalParameterValue("am1"),
                  getInternalParameterValue("am2"),
                  getInternalParameterValue("am1"));

    mAMEnv.lengths(getInternalParameterValue("amRise"),
                   0.001,
                   getInternalParameterValue("amRise"));

    mPan.pos(getInternalParameterValue("pan"));

    mAmpEnv.reset();
    mAMEnv.reset();
    // Map table number to table in memory
    switch (int(getInternalParameterValue("amFunc"))) {
    case 0: mOsc.source(tbSin); break;
    case 1: mOsc.source(tbSqr); break;
    case 2: mOsc.source(tbPls); break;
    case 3: mOsc.source(tbDin); break;
    }
  }

};



class MyApp : public App
{
public:

  virtual void onInit( ) override {
    addWave(tbSin, SINE);
    addWave(tbSqr, SQUARE);
    addWave(tbPls, IMPULSE, 4);

    // inharmonic partials
    {    float A[] = {1, 0.7, 0.45, 0.3, 0.15, 0.08};
      float C[] = {10, 27, 54, 81, 108, 135};
      addSines(tbDin, A,C,6);
    }

  }

    virtual void onCreate() override {
        ParameterGUI::initialize();

        // Play example sequence. Comment this line to start from scratch
//        synthManager.synthSequencer().playSequence("synth6.synthSequence");
        synthManager.synthRecorder().verbose(true);
    }

    virtual void onSound(AudioIOData &io) override {
        synthManager.render(io); // Render audio
    }

    virtual void onDraw(Graphics &g) override {
        g.clear();
        synthManager.render(g);

        // Draw GUI
        ParameterGUI::beginDraw();
        ParameterGUI::beginPanel(synthManager.name());
        synthManager.drawSynthWidgets();
        ParameterGUI::endPanel();
        ParameterGUI::endDraw();
    }

    virtual void onKeyDown(Keyboard const& k) override {
      if (ParameterGUI::usingKeyboard()) { //Ignore keys if GUI is using them
        return;
      }
        if (k.shift()) {
            // If shift pressed then keyboard sets preset
            int presetNumber = asciiToIndex(k.key());
            synthManager.recallPreset(presetNumber);
        } else {
            // Otherwise trigger note for polyphonic synth
            int midiNote = asciiToMIDI(k.key());
            if (midiNote > 0) {
              synthManager.voice()->setInternalParameterValue("frequency", ::pow(2.f, (midiNote - 69.f)/12.f) * 432.f);
              synthManager.triggerOn(midiNote);
            }
        }
    }

    virtual void onKeyUp(Keyboard const& k) override {
        int midiNote = asciiToMIDI(k.key());
        if (midiNote > 0) {
            synthManager.triggerOff(midiNote);
        }
    }

    void onExit() override {
        ParameterGUI::cleanup();
    }

    SynthGUIManager<OscAM> synthManager {"synth6"};
};


//SynthSequencer s;
//s.add<OscAM>( 0).set(5, 262, 0.5, 0.1,0.08,0.8, 0.2,0.8,0.5, 1.4870, tbSin);
//s.add<OscAM>( 5).set(10, 262, 0.5, 0.1,0.08,0.8, 0.0,1.0,0.0, 2.0001, tbSin);
//s.add<OscAM>(15).set(5, 262, 0.5, 0.1,0.08,0.8, 0.2,0.8,1.0, 2.0001, tbPls);
//s.add<OscAM>(20).set(5, 262, 0.5, 0.1,0.08,0.8, 0.2,0.8,1.0, 2.0001/10., tbDin);



int main(){
  MyApp app;

  app.navControl().active(false); // Disable navigation via keyboard, since we will be using keyboard for note triggering

  // Set up audio
  app.initAudio(48000., 256, 2, 0);
  // Set sampling rate for Gamma objects from app's audio
  gam::sampleRate(app.audioIO().framesPerSecond());
  app.audioIO().print();

  app.start();

  return 0;
}
