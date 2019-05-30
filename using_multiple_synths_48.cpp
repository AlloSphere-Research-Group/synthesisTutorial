/*    Gamma - Generic processing library
    See COPYRIGHT file for authors and license information
    Example: Synth 4-FM + 8-Subtractive synth. 
    Description: Synthesizer FM. with vibrato + Subtractive 
    Author: Myungin Lee (Ben)

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
    gam::ADSR<> mVibEnv;
    
    gam::Sine<> car, mod, mVib;    // carrier, modulator sine oscillators

    // Additional members
    Mesh mMesh;
    float mDur;
    float mModAmt = 50;
    float mVibFrq;
    float mVibDepth;
    float mVibRise;

    void init() override {
//      mAmpEnv.curve(0); // linear segments

      mAmpEnv.levels(0,1,1,0);
      mModEnv.levels(0,1,1,0);
      mVibEnv.levels(0,1,1,0);
//      mVibEnv.curve(0);

      // We have the mesh be a sphere
      addDisc(mMesh, 1.0, 30);

      createInternalTriggerParameter("dur", 2, 0, 10);
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

      createInternalTriggerParameter("vibRate1", 0.01, 0.0, 10.0);
      createInternalTriggerParameter("vibRate2", 0.5, 0.0, 10.0);
      createInternalTriggerParameter("vibRise", 0, 0.0, 10.0);
      createInternalTriggerParameter("vibDepth", 0, 0.0, 10.0);

      createInternalTriggerParameter("pan", 0.0, -1.0, 1.0);
    }

    //
    virtual void onProcess(AudioIOData& io) override {
        updateFromParameters();
        mVib.freq(mVibEnv());
        float carBaseFreq = getInternalParameterValue("freq")*getInternalParameterValue("carMul");
        float modScale = getInternalParameterValue("freq") * getInternalParameterValue("modMul");
        float amp = getInternalParameterValue("amplitude");
        while(io()){
          mVib.freq(mVibEnv());
          car.freq( (1+ mVib()*mVibDepth)*carBaseFreq + mod()*mModEnv()*modScale);
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
        g.translate(getInternalParameterValue("freq")/ 300 - 2,  (getInternalParameterValue("idx3") + getInternalParameterValue("idx2"))/15-1, -4);
        float scaling = getInternalParameterValue("amplitude")/3;
        g.scale(scaling, scaling , scaling* 1);
        g.color(HSV( getInternalParameterValue("modMul")/20, 1, mEnvFollow.value()* 10));
        g.draw(mMesh);
        g.popMatrix();
    }
    
    virtual void onTriggerOn() override {
        updateFromParameters();

        float modFreq = getInternalParameterValue("freq") * getInternalParameterValue("modMul");
        mod.freq(modFreq);
    
        mVibEnv.lengths()[0] = mDur * (1-mVibRise);
        mVibEnv.lengths()[1] = mDur * mVibRise;
        mAmpEnv.reset();
        mVibEnv.reset();
        mModEnv.reset();
    }
    virtual void onTriggerOff() override {
        mAmpEnv.triggerRelease();
        mModEnv.triggerRelease();
        mVibEnv.triggerRelease();

    }

    void updateFromParameters() {
      mModEnv.levels()[0]= getInternalParameterValue("idx1");
      mModEnv.levels()[1]= getInternalParameterValue("idx2");
      mModEnv.levels()[2]= getInternalParameterValue("idx2");
      mModEnv.levels()[3]= getInternalParameterValue("idx3");

      mAmpEnv.levels()[1] = 1.0;
      //mAmpEnv.levels()[2] = 1.0;
      mAmpEnv.levels()[2] = getInternalParameterValue("sustain");
      
      mAmpEnv.lengths()[0] = getInternalParameterValue("attackTime");
      mModEnv.lengths()[0] = getInternalParameterValue("attackTime");

      mAmpEnv.lengths()[3] = getInternalParameterValue("releaseTime");
      mModEnv.lengths()[3] = getInternalParameterValue("releaseTime");
    
      mAmpEnv.totalLength(getInternalParameterValue("dur"), 1);  
      mModEnv.lengths()[1] = mAmpEnv.lengths()[1];
      mVibEnv.levels()[1]=getInternalParameterValue("vibRate1");
      mVibEnv.levels()[2]=getInternalParameterValue("vibRate2");
      mVibDepth=getInternalParameterValue("vibDepth");
      mVibRise= getInternalParameterValue("vibRise");
    }
};


class Sub : public SynthVoice {
public:

    // Unit generators
    float mNoiseMix;
    gam::Pan<> mPan;
    gam::ADSR<> mAmpEnv;
    gam::EnvFollow<> mEnvFollow;  // envelope follower to connect audio output to graphics
    gam::DSF<> mOsc;
    gam::NoiseWhite<> mNoise;
    gam::Reson<> mRes;
    gam::Env<2> mCFEnv;
    gam::Env<2> mBWEnv;
    // Additional members
    Mesh mMesh;

    // Initialize voice. This function will nly be called once per voice
    void init() override {
        mAmpEnv.curve(0); // linear segments
        mAmpEnv.levels(0,1.0,1.0,0); // These tables are not normalized, so scale to 0.3
        mAmpEnv.sustainPoint(2); // Make point 2 sustain until a release is issued
        mCFEnv.curve(0);
        mBWEnv.curve(0);
        mOsc.harmonics(12);
        // We have the mesh be a sphere
        addDisc(mMesh, 1.0, 30);

        createInternalTriggerParameter("amplitude", 0.3, 0.0, 1.0);
        createInternalTriggerParameter("frequency", 60, 20, 5000);
        createInternalTriggerParameter("attackTime", 0.1, 0.01, 3.0);
        createInternalTriggerParameter("releaseTime", 3.0, 0.1, 10.0);
        createInternalTriggerParameter("sustain", 0.7, 0.0, 1.0);
        createInternalTriggerParameter("curve", 4.0, -10.0, 10.0);
        createInternalTriggerParameter("noise", 0.0, 0.0, 1.0);
        createInternalTriggerParameter("envDur", 0.0, 5.0);
        createInternalTriggerParameter("cf1", 10.0, 10.0, 5000);
        createInternalTriggerParameter("cf2", 10.0, 10.0, 5000);
        createInternalTriggerParameter("cfRise", 0.5, 0.1, 2);
        createInternalTriggerParameter("bw1", 10.0, 10.0, 5000);
        createInternalTriggerParameter("bw2", 10.0, 10.0, 5000);
        createInternalTriggerParameter("bwRise", 0.5, 0.1, 2);
        createInternalTriggerParameter("hmnum", 12.0, 5.0, 20.0);
        createInternalTriggerParameter("hmamp", 1.0, 0.0, 1.0);
        createInternalTriggerParameter("pan", 0.0, -1.0, 1.0);

    }

    //
    
    virtual void onProcess(AudioIOData& io) override {
        updateFromParameters();
        float amp = getInternalParameterValue("amplitude");
        float noiseMix = getInternalParameterValue("noise");
        while(io()){
            // mix oscillator with noise
            float s1 = mOsc()*(1-noiseMix) + mNoise()*noiseMix;

            // apply resonant filter
            mRes.set(mCFEnv(), mBWEnv());
            s1 = mRes(s1);

            // appy amplitude envelope
            s1 *= mAmpEnv() * amp;

            float s2;
            mPan(s1, s1,s2);
            io.out(0) += s1;
            io.out(1) += s2;
        }
        
        
        if(mAmpEnv.done() && (mEnvFollow.value() < 0.001f)) free();
    }

 /*   virtual void onProcess(Graphics &g) {
        float frequency = getInternalParameterValue("frequency");
        float amplitude = getInternalParameterValue("amplitude");
        g.pushMatrix();
        g.translate(amplitude,  amplitude, -4);
        g.scale(frequency/200, frequency/400, 1);
        g.color(mEnvFollow.value(), frequency/1000, mEnvFollow.value()* 10, 0.4);
        g.draw(mMesh);
        g.popMatrix();
    }
*/
    virtual void onTriggerOn() override {
        updateFromParameters();
        mAmpEnv.reset();
        mCFEnv.reset();
        mBWEnv.reset();
        
    }

    virtual void onTriggerOff() override {
        mAmpEnv.triggerRelease();
    }

    void updateFromParameters() {
        mOsc.freq(getInternalParameterValue("frequency"));
        mOsc.harmonics(getInternalParameterValue("hmnum"));
        mOsc.ampRatio(getInternalParameterValue("hmamp"));
        mAmpEnv.attack(getInternalParameterValue("attackTime"));
        mAmpEnv.release(getInternalParameterValue("releaseTime"));
        mAmpEnv.levels()[1]=getInternalParameterValue("sustain");
        mAmpEnv.levels()[2]=getInternalParameterValue("sustain");

        mAmpEnv.curve(getInternalParameterValue("curve"));
        mPan.pos(getInternalParameterValue("pan"));
        mCFEnv.levels(getInternalParameterValue("cf1"),
                      getInternalParameterValue("cf2"),
                      getInternalParameterValue("cf1"));


        mCFEnv.lengths()[0] = getInternalParameterValue("cfRise");
        mCFEnv.lengths()[1] = 1 - getInternalParameterValue("cfRise");
        mBWEnv.levels(getInternalParameterValue("bw1"),
                      getInternalParameterValue("bw2"),
                      getInternalParameterValue("bw1"));
        mBWEnv.lengths()[0] = getInternalParameterValue("bwRise");
        mBWEnv.lengths()[1] = 1- getInternalParameterValue("bwRise");

        mCFEnv.totalLength(getInternalParameterValue("envDur"));
        mBWEnv.totalLength(getInternalParameterValue("envDur"));
    }
};


class MyApp : public App    
{
public:
//    ParameterMIDI parameterMIDI;
    int midiNote;
    virtual void onCreate() override {
        ParameterGUI::initialize();

        // Play example sequence. Comment this line to start from scratch
    //    synthManager.synthSequencer().playSequence("synth2.synthSequence");
        synthManager.synthRecorder().verbose(true);
        // This line registers the Sub voice with the synthManager that already uses Sub
        synthManager.synth().registerSynthClass<FM>();        
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
            if(k.ctrl()) {
              midiNote += 24;
            }
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
    SynthGUIManager<Sub> synthManager {"Sub_FM"};
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
