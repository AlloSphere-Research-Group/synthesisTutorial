/*    Gamma - Generic processing library
    See COPYRIGHT file for authors and license information
    Example: Synth 4FM. 
    Description: Synthesis Tutorial 4. FM. 
    Author: Myungin Lee (Ben)
    
#    amplitude attackTime curve releaseTime carrierFrequency modEnv_Attack modEnv_Curve modEnv_Release carMul modMul modAmt pan
@ 0 5 FM 0.5 0.1 0.75 0.1 262 0.01 7 5 1 1.0007 1 1 #brass
@ 5 5 FM 0.5 0.1 0.75 0.1 262 0.01 4 4 3 2.0007 1 -1 #clarinet
@ 10 5 FM 0.5 0.2 0.75 0.1 262 2 2 2 3 1.0007 1 0 #oboe
@ 15 5 FM 0.5 0.2 0.75 0.1 139 0.01 1.5 5 5 1.0007 1 0 #bassoon
@ 20 10 FM 0.5 0.001 0.8 9.9 100 7 7 7 1 1.4 1 0 #gong
@ 30 0.3 FM 0.5 0.001 0.8 0.25 100 1 1 1 1 1.48 1 0 #drum

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


//using namespace gam;
using namespace al;
using namespace std;


class FM : public SynthVoice {
public:
    // Unit generators
    gam::Pan<> mPan;
    gam::ADSR<> mAmpEnv;
    gam::ADSR<> mModEnv;
    gam::EnvFollow<> mEnvFollow;

//    gam::Sine<> mVib;
//    gam::Env<2> mVibEnv;
    
    gam::Sine<> car, mod;    // carrier, modulator sine oscillators

    // Additional members
    Mesh mMesh;

    void init() override {
//      mAmpEnv.curve(0); // linear segments
      mAmpEnv.levels(0,1,1,0);

      mModEnv.levels(0,1, 0);

      // We have the mesh be a sphere
      addDisc(mMesh, 1.0, 30);

      createInternalTriggerParameter("freq", 440, 10, 4000.0);
      createInternalTriggerParameter("amplitude", 0.5, 0.0, 1.0);
      createInternalTriggerParameter("attackTime", 0.1, 0.01, 3.0);
      createInternalTriggerParameter("releaseTime", 0.1, 0.1, 10.0);
      createInternalTriggerParameter("sustain", 0.75, 0.1, 1.0);

      // FM index
      createInternalTriggerParameter("idx1", 0.01, 0.0, 10.0);
      createInternalTriggerParameter("idx2", 7, 0.0, 10.0);
      createInternalTriggerParameter("idx3", 5, 0.0, 10.0);

      createInternalTriggerParameter("carMul", 1, 0.0, 20.0);
      createInternalTriggerParameter("modMul", 1.0007, 0.0, 20.0);

      createInternalTriggerParameter("pan", 0.0, -1.0, 1.0);
    }

    //
    virtual void onProcess(AudioIOData& io) override {
        updateFromParameters();
        float carBaseFreq = getInternalParameterValue("freq")*getInternalParameterValue("carMul");
        float modScale = getInternalParameterValue("freq") * getInternalParameterValue("modMul");
        float amp = getInternalParameterValue("amplitude");
        while(io()){
          car.freq(carBaseFreq + mod()*mModEnv()*modScale);
          float s1 = car() * mAmpEnv() * amp;
          float s2;
          mEnvFollow(s1);
          mPan(s1, s1,s2);
          io.out(0) += s1;
          io.out(1) += s2;
        }
        if(mAmpEnv.done() && (mEnvFollow.value() < 0.001)) free();
    }

    virtual void onProcess(Graphics &g) {
        g.pushMatrix();
        g.translate(getInternalParameterValue("freq")/ 300 - 2,  getInternalParameterValue("modAmt")/25-1, -4);
        float scaling = getInternalParameterValue("amplitude")*1;
        g.scale(scaling, scaling , scaling* 1);
        g.color(HSV( getInternalParameterValue("modMul")/20, 1, mEnvFollow.value()* 10));
        g.draw(mMesh);
        g.popMatrix();
    }
    
    virtual void onTriggerOn() override {
        updateFromParameters();

        float modFreq = getInternalParameterValue("freq") * getInternalParameterValue("modMul");
        mod.freq(modFreq);

//        mModEnv.lengths()[1] = mAmpEnv.lengths()[1];

        mAmpEnv.reset();
        mModEnv.reset();
    }
    virtual void onTriggerOff() override {
        mAmpEnv.triggerRelease();
    }

    void updateFromParameters() {
      mModEnv.levels()[0]= getInternalParameterValue("idx1");
      mModEnv.levels()[1]= getInternalParameterValue("idx2");
      mModEnv.levels()[2]= getInternalParameterValue("idx2");
      mModEnv.levels()[3]= getInternalParameterValue("idx3");

      mAmpEnv.levels()[1] = 1.0;
      mAmpEnv.levels()[2] = 1.0;

      mAmpEnv.lengths()[0] = getInternalParameterValue("attackTime");
      mModEnv.lengths()[0] = getInternalParameterValue("attackTime");

      mAmpEnv.lengths()[3] = getInternalParameterValue("releaseTime");
      mModEnv.lengths()[3] = getInternalParameterValue("releaseTime");

    }

};


class MyApp : public App    
{
public:
    SynthGUIManager<FM> synthManager {"synth4"};

    ParameterMIDI parameterMIDI;
    int midiNote;
    virtual void onCreate() override {
        ParameterGUI::initialize();

        // Play example sequence. Comment this line to start from scratch
    //    synthManager.synthSequencer().playSequence("synth2.synthSequence");
        synthManager.synthRecorder().verbose(true);

        parameterMIDI.open(0);    
        parameterMIDI.connectControl(synthManager.voice()->getInternalParameter("pan"), 10, 1);
//        parameterMIDI.connectNoteToValue(synthManager.voice()->getInternalParameter("carrierFrequency"),0,0,127,127);
//  void connectNoteToValue(Parameter &param, int channel, float min, int low, float max = -1, int high = -1) 
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
              synthManager.voice()->setInternalParameterValue("freq", ::pow(2.f, (midiNote - 69.f)/12.f) * 432.f);
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

};


int main(){
    MyApp app;

    app.navControl().active(false); // Disable navigation via keyboard, since we will be using keyboard for note triggering

    // Set up audio
    app.initAudio(48000., 256, 2, 0);
    // Set sampling rate for Gamma objects from app's audio
    gam::sampleRate(app.audioIO().framesPerSecond());
    app.audioIO().print();

    app.start();

}
