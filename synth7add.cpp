
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

class AddSyn : public SynthVoice {
public:

  Sine<> mOsc;
  Sine<> mOsc1;
  Sine<> mOsc2;
  Sine<> mOsc3;
  Sine<> mOsc4;
  Sine<> mOsc5;
  Sine<> mOsc6;
  Sine<> mOsc7;
  Sine<> mOsc8;
  Sine<> mOsc9;
  ADSR<> mEnvStri;
  ADSR<> mEnvLow;
  ADSR<> mEnvUp;
  Pan<> mPan;
  EnvFollow<> mEnvFollow;

  // Additional members
  Mesh mMesh;

  virtual void init() {

    // Intialize envelopes
    mEnvStri.curve(-4); // make segments lines
    mEnvStri.levels(0,1,1,0);
    mEnvStri.lengths(0.1, 0.1, 0.1);
    mEnvStri.sustain(2); // Make point 2 sustain until a release is issued
    mEnvLow.curve(-4); // make segments lines
    mEnvLow.levels(0,1,1,0);
    mEnvLow.lengths(0.1, 0.1, 0.1);
    mEnvLow.sustain(2); // Make point 2 sustain until a release is issued
    mEnvUp.curve(-4); // make segments lines
    mEnvUp.levels(0,1,1,0);
    mEnvUp.lengths(0.1, 0.1, 0.1);
    mEnvUp.sustain(2); // Make point 2 sustain until a release is issued

    // We have the mesh be a sphere
    addDisc(mMesh, 1.0, 30);

    createInternalTriggerParameter("amp", 0.01, 0.0, 0.3);
    createInternalTriggerParameter("freq", 60, 20, 5000);
    createInternalTriggerParameter("ampStri", 0.5, 0.0, 1.0);
    createInternalTriggerParameter("attackStri", 0.1, 0.01, 3.0);
    createInternalTriggerParameter("releaseStri", 0.1, 0.1, 10.0);
    createInternalTriggerParameter("sustainStri", 0.8, 0.0, 1.0);
    createInternalTriggerParameter("ampLow", 0.5, 0.0, 1.0);
    createInternalTriggerParameter("attackLow", 0.001, 0.01, 3.0);
    createInternalTriggerParameter("releaseLow", 0.1, 0.1, 10.0);
    createInternalTriggerParameter("sustainLow", 0.8, 0.0, 1.0);
    createInternalTriggerParameter("ampUp", 0.6, 0.0, 1.0);
    createInternalTriggerParameter("attackUp", 0.01, 0.01, 3.0);
    createInternalTriggerParameter("releaseUp", 0.075, 0.1, 10.0);
    createInternalTriggerParameter("sustainUp", 0.9, 0.0, 1.0);
    createInternalTriggerParameter("freqStri1", 1.0, 0.1, 10);
    createInternalTriggerParameter("freqStri2", 2.001, 0.1, 10);
    createInternalTriggerParameter("freqStri3", 3.0, 0.1, 10);
    createInternalTriggerParameter("freqLow1", 4.009, 0.1, 10);
    createInternalTriggerParameter("freqLow2", 5.002, 0.1, 10);
    createInternalTriggerParameter("freqUp1", 6.0, 0.1, 10);
    createInternalTriggerParameter("freqUp2", 7.0, 0.1, 10);
    createInternalTriggerParameter("freqUp3", 8.0, 0.1, 10);
    createInternalTriggerParameter("freqUp4", 9.0, 0.1, 10);
    createInternalTriggerParameter("pan", 0.0, -1.0, 1.0);
  }

  virtual void onProcess(AudioIOData& io) override {
    // Parameters will update values once per audio callback
    float freq = getInternalParameterValue("freq");
    mOsc.freq(freq);
    mOsc1.freq(getInternalParameterValue("freqStri1") * freq);
    mOsc2.freq(getInternalParameterValue("freqStri2") * freq);
    mOsc3.freq(getInternalParameterValue("freqStri3") * freq);
    mOsc4.freq(getInternalParameterValue("freqLow1") * freq);
    mOsc5.freq(getInternalParameterValue("freqLow2") * freq);
    mOsc6.freq(getInternalParameterValue("freqUp1") * freq);
    mOsc7.freq(getInternalParameterValue("freqUp2") * freq);
    mOsc8.freq(getInternalParameterValue("freqUp3") * freq);
    mOsc9.freq(getInternalParameterValue("freqUp4") * freq);
    mPan.pos(getInternalParameterValue("pan"));
    float ampStri = getInternalParameterValue("ampStri");
    float ampUp = getInternalParameterValue("ampUp");
    float ampLow = getInternalParameterValue("ampLow");
    float amp = getInternalParameterValue("amp");
    while(io()){
      float s1 = (mOsc1() + mOsc2() + mOsc3()) * mEnvStri() * ampStri;
      s1 += (mOsc4() + mOsc5()) * mEnvLow() * ampLow;
      s1 += (mOsc6() + mOsc7() + mOsc8() + mOsc9()) * mEnvUp() * ampUp;
      s1 *= amp;
      float s2;
      mEnvFollow(s1);
      mPan(s1, s1,s2);
      io.out(0) += s1;
      io.out(1) += s2;
    }
    //if(mEnvStri.done()) free();
    if(mEnvStri.done() && mEnvUp.done() && mEnvLow.done() && (mEnvFollow.value() < 0.001)) free();
  }

  virtual void onTriggerOn() override {

    mEnvStri.attack(getInternalParameterValue("attackStri"));
    mEnvStri.decay(getInternalParameterValue("attackStri"));
    mEnvStri.sustain(getInternalParameterValue("sustainStri"));
    mEnvStri.release(getInternalParameterValue("releaseStri"));

    mEnvLow.attack(getInternalParameterValue("attackLow"));
    mEnvLow.decay(getInternalParameterValue("attackLow"));
    mEnvLow.sustain(getInternalParameterValue("sustainLow"));
    mEnvLow.release(getInternalParameterValue("releaseLow"));

    mEnvUp.attack(getInternalParameterValue("attackUp"));
    mEnvUp.decay(getInternalParameterValue("attackUp"));
    mEnvUp.sustain(getInternalParameterValue("sustainUp"));
    mEnvUp.release(getInternalParameterValue("releaseUp"));

    mPan.pos(getInternalParameterValue("pan"));

    mEnvStri.reset();
    mEnvLow.reset();
    mEnvUp.reset();
  }

  virtual void onTriggerOff() override {
    std::cout << "trigger off" <<std::endl;
    mEnvStri.triggerRelease();
    mEnvLow.triggerRelease();
    mEnvUp.triggerRelease();
  }


};

// Commented out below are things that still need to be ported to this new version

//float myScale[6];
//void initScaleToMyPitches() {
//    myScale[0] = 100;
//    myScale[1] = 121.245;
//    myScale[2] = 145.876234;
//    myScale[3] = 167.4786234;
//    myScale[4] = 367.4786234;
//    myScale[5] = 1367.4786234;
//}

//float randomFromMyScale() {
//    int index = rnd::uni(0,6);
//    std::cout << "index " << index << " is " << myScale[index] << std::endl;
//    return myScale[index];
//}


//int main(){
//    // initScaleTo12TET(110);
//    initScaleToMyPitches();
//    initScaleTo12TET(110);

//    SynthSequencer s;

//    s.add<AddSyn>(0).set(6.2,155.6,0.1, 0.5,0.0001,3.8,0.3, 0.4,0.0001,6.0,0.99,  0.3,0.0001,6.0,0.9,2,3,4.07,0.56,0.92,1.19,1.7,2.75,3.36);

//    s.add<AddSyn>(7.0).set(6.2,622.2,0.1,0.5,0.0001,6.1,0.99,0.4,0.0005,6.1,0.99,0.3,0.0005,6.1,0.9,2,3,4.07,0.56,0.92,1.19,1.7,2.75,3.36);
//    s.add<AddSyn>(14.0).set(6.2,155.6,0.01,0.5,0.1,0.1,0.8,0.5,0.001,0.1,0.8,0.6,0.01,0.075,0.9,1,2.001,3,4.00009,5.0002,6,7,8,9);
//    s.add<AddSyn>(21.0).set(6.2,77.78,0.01,0.5,0.1,0.4,0.8,0.5,0.001,0.4,0.8,0.6,0.01,0.4,0.5,1,2.0001,3,4.00009,5.0002,6,7,8,9);
//    s.add<AddSyn>(28.0).set(6.2,311.1,0.01,0.5,0.1,0.4,0.8,0.5,0.001,0.4,0.8,0.6,0.01,0.4,0.5,1,1.0001,3,3.0009,5.0002,5,7,7.0009,9);
//    s.add<AddSyn>(35.0).set(6.2,1245,0.01,0.5,0.0001,6.1,0.99,0.4,0.0005,6.1,0.99,0.3,0.0005,6.1,0.9,1,3,4.07,.56,.92,1.19,1.7,2.74,3.36);
//    // s.add<Chimes>(0).freq(rnd::uni(.2,.5), rnd::uni(50.0f,500.0f));
//    //fillTime(s,40,44, .1, .5, 1000, 2000);
//    //fillTime(s,40,44, 1, 5, 1000, 2000);
//    //fillTime(s,40,44, .5, 1.5, 200, 500);
//    //fillTimeWith12TET(s,45,58,.100,.400);
//    //fillTime(s,40,44, 0.0001, 0.1, 0.0001, 0.1, 0.0001, 0.1, 200, 500);
//    //fillTimeWith12TET(s,45,58,0.0001, 0.1, 0.0001, 0.1, 0.0001, 0.1);
//     fillTime(s,40,44, 0.0001, 0.0001, 0.0001, 0.1, 0.1, 0.1, 200, 500);
//     fillTimeWith12TET(s,45,58,0.0001, 0.0001, 0.0001, 0.1, 0.1, 0.1);
//    //fillTimeWith12TET(s,45,58,1,4);
//    s.add<AddSyn>(59.0).freq(6.3, 155.6);
//    s.add<AddSyn>(65.0).freq(6.3, randomFromMyScale());
//    s.add<AddSyn>(71.0).freq(7.0,311.1).freqUp(2., 3., 4., 5.).decayLow(4.56);
//    s.add<Chimes>(77.0).freq(6.3,1245);
//    s.add<AddSyn>(80.0).freq(6.3, randomFrom12TET());

//    // A bunch of random-freq notes over 10 seconds:
//    //for (float t = 0; t<10; t += 0.5) {
//    //    s.add<Chimes>(t).dur(0.4).freq(randomFromHarmonicSeries());
//    //}


class MyApp : public App
{
public:

  // GUI manager for SineEnv voices
  // The name provided determines the name of the directory
  // where the presets and sequences are stored
  SynthGUIManager<AddSyn> synthManager {"synth7"};

  float harmonicSeriesScale[20];

  float halfStepScale[20];
  float halfStepInterval = 1.05946309; // 2^(1/12)


  // This function is called right after the window is created
  // It provides a grphics context to initialize ParameterGUI
  // It's also a good place to put things that should
  // happen once at startup.
  virtual void onCreate() override {
    ParameterGUI::initialize();

    initScaleToHarmonicSeries();
    initScaleTo12TET(110);

    // Play example sequence. Comment this line to start from scratch
    synthManager.synthSequencer().playSequence("synth7.synthSequence");
    synthManager.synthRecorder().verbose(true);
  }

  // The audio callback function. Called when audio hardware requires data
  virtual void onSound(AudioIOData &io) override {
    synthManager.render(io); // Render audio
  }

  // The graphics callback function.
  virtual void onDraw(Graphics &g) override {
    g.clear();
    synthManager.render(g);

    // Draw GUI
    ParameterGUI::beginDraw();
    ParameterGUI::beginPanel("AddSyn");

    if (ImGui::Button("Chimes")) {
      fillTime(0,4, 0.0001, 0.0001, 0.0001, 0.1, 0.1, 0.1, 200, 500);
    }
    ImGui::SameLine();
    if (ImGui::Button("Chimes 12TET")) {
      fillTimeWith12TET(0,4, 0.0001, 0.0001, 0.0001, 0.1, 0.1, 0.1);
    }
    ImGui::Separator();
    synthManager.drawSynthWidgets();

    ParameterGUI::endPanel();
    ParameterGUI::endDraw();
  }

  // Whenever a key is pressed, this function is called
  virtual void onKeyDown(Keyboard const& k) override {
    if (ParameterGUI::usingKeyboard()) { // Ignore keys if GUI is using keyboard
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

  // Whenever a key is released this function is called
  virtual void onKeyUp(Keyboard const& k) override {
    int midiNote = asciiToMIDI(k.key());
    if (midiNote > 0) {
      synthManager.triggerOff(midiNote);
    }
  }

  void onExit() override {
    ParameterGUI::cleanup();
  }

  void initScaleToHarmonicSeries() {
    for (int i=0;i<20;++i) {
      harmonicSeriesScale[i] = 100*i;
    }
  }

  void initScaleTo12TET(float lowest) {
    float f = lowest;
    for (int i=0;i<20;++i) {
      halfStepScale[i] = f;
      f *= halfStepInterval;
    }
  }

  float randomFrom12TET() {
    int index = rnd::uni(0,20);
    // std::cout << "index " << index << " is " << myScale[index] << std::endl;
    return halfStepScale[index];
  }

  float randomFromHarmonicSeries() {
      int index = rnd::uni(0,20);
      // std::cout << "index " << index << " is " << myScale[index] << std::endl;
      return harmonicSeriesScale[index];
  }

  void fillTime(float from, float to, float minattackStri, float minattackLow, float minattackUp, float maxattackStri, float maxattackLow, float maxattackUp, float minFreq, float maxFreq) {
        while (from <= to) {
            float nextAtt = rnd::uni((minattackStri+minattackLow+minattackUp),(maxattackStri+maxattackLow+maxattackUp));
            auto *voice = synthManager.synth().getVoice<AddSyn>();
            voice->setTriggerParams({0.03,440, 0.5,0.0001,3.8,0.3,   0.4,0.0001,6.0,0.99,  0.3,0.0001,6.0,0.9,  2,3,4.07,0.56,0.92,1.19,1.7,2.75,3.36, 0.0});
            voice->setInternalParameterValue("attackStr", nextAtt);
            voice->setInternalParameterValue("freq", rnd::uni(minFreq,maxFreq));
            synthManager.synthSequencer().addVoiceFromNow(voice, from, 0.2);
            std::cout << "old from " << from << " plus nextnextAtt " << nextAtt << std::endl;
            from += nextAtt;
        }
  }

  void fillTimeWith12TET(float from, float to, float minattackStri, float minattackLow, float minattackUp, float maxattackStri, float maxattackLow, float maxattackUp) {
      while (from <= to) {

        float nextAtt = rnd::uni((minattackStri+minattackLow+minattackUp),(maxattackStri+maxattackLow+maxattackUp));
        auto *voice = synthManager.synth().getVoice<AddSyn>();
        voice->setTriggerParams({0.03,440, 0.5,0.0001,3.8,0.3,   0.4,0.0001,6.0,0.99,  0.3,0.0001,6.0,0.9,  2,3,4.07,0.56,0.92,1.19,1.7,2.75,3.36, 0.0});
        voice->setInternalParameterValue("attackStr", nextAtt);
        voice->setInternalParameterValue("freq", randomFrom12TET());
        synthManager.synthSequencer().addVoiceFromNow(voice, from, 0.2);
        std::cout << "12 old from " << from << " plus nextAtt " << nextAtt << std::endl;
        from += nextAtt;
      }
  }

};


int main(){    // Create app instance
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
