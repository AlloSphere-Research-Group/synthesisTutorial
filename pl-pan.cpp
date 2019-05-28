/*    Gamma - Generic processing library
    See COPYRIGHT file for authors and license information

    Example:        Filter / Plucked String
    Description:    Simulation of a plucked string with noise and a feedback
                    delay-line.
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

using namespace al;

class PluckedString : public SynthVoice {
public:
    float mAmp;
    float mDur;
    float mPanRise;
    gam::Pan<> mPan;
    gam::NoiseWhite<> noise;
    gam::Decay<> env;
    gam::MovingAvg<> fil {2};
    gam::Delay<float, gam::ipl::Trunc> delay;
    gam::ADSR<> mAmpEnv;
    gam::EnvFollow<> mEnvFollow;
    gam::Env<2> mPanEnv;

    // Additional members
    Mesh mMesh;


    virtual void init(){
        mAmp  = 1;
        mDur = 2;
        mAmpEnv.curve(4); // make segments lines
        mAmpEnv.levels(1,1,0);
        mPanEnv.curve(4);
        env.decay(0.1);
//      delay(1./27.5, 1./frq) ??? Delay(float maxDelay, float delay)
        delay.maxDelay(1./27.5);
        delay.delay(1./440.0);


        addDisc(mMesh, 1.0, 30);
        createInternalTriggerParameter("amplitude", 0.1, 0.0, 1.0);
        createInternalTriggerParameter("frequency", 60, 20, 5000);
        createInternalTriggerParameter("decay", 0.1, 0.1, 10.0);
        createInternalTriggerParameter("attackTime", 0.001, 0.001, 1.0);
        createInternalTriggerParameter("releaseTime", 3.0, 0.1, 10.0);
        createInternalTriggerParameter("sustain", 0.7, 0.0, 1.0);
        createInternalTriggerParameter("curve", 4.0, -10.0, 10.0);
        createInternalTriggerParameter("Pan1", 0.0, -1.0, 1.0);
        createInternalTriggerParameter("Pan2", 0.0, -1.0, 1.0);
        createInternalTriggerParameter("PanRise", 0.0, -1.0, 1.0); // range check
    }
    
//    void reset(){ env.reset(); }

    float operator() (){
        return (*this)(noise()*env());
    }
    float operator() (float in){
        return delay(
                     fil( delay() + in )
                     );
    }

    virtual void onProcess(AudioIOData& io) override {

        while(io()){
            mPan.pos(mPanEnv());
            float s1 =  (*this)() * mAmpEnv() * mAmp;
            float s2;
            mEnvFollow(s1);
            mPan(s1, s1,s2);
            io.out(0) += s1;
            io.out(1) += s2;
        }
        if(mAmpEnv.done() && (mEnvFollow.value() < 0.001)) free();

    }
/*
virtual void onProcess(Graphics &g) {
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
        env.reset();
        delay.zero();
    }

    virtual void onTriggerOff() override {
        mAmpEnv.triggerRelease();
    }

    void updateFromParameters() {
        mPanEnv.levels(getInternalParameterValue("Pan1"),
                       getInternalParameterValue("Pan2"),
                       getInternalParameterValue("Pan1"));
        mPanRise = getInternalParameterValue("PanRise");
        delay.freq(getInternalParameterValue("frequency"));
        mAmp = getInternalParameterValue("amplitude");
        mAmpEnv.attack(getInternalParameterValue("attackTime"));
        mAmpEnv.decay(getInternalParameterValue("attackTime"));
//        mAmpEnv.decay(getInternalParameterValue("decay"));
        mAmpEnv.sustain(getInternalParameterValue("sustain"));
        mAmpEnv.release(getInternalParameterValue("releaseTime"));


        mPanEnv.lengths()[0] = mDur * (1-mPanRise);
        mPanEnv.lengths()[1] = mDur * mPanRise;
    }

};

class MyApp : public App    
{
public:
    virtual void onCreate() override {
        ParameterGUI::initialize();

        // Play example sequence. Comment this line to start from scratch
    //    synthManager.synthSequencer().playSequence("pl-pan.synthSequence");
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
            if(k.ctrl()) {
              midiNote -= 24;
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
            synthManager.triggerOff(midiNote -24); // Trigger both off for safety
        }
    }

    void onExit() override {
        ParameterGUI::cleanup();
    }
    SynthGUIManager<PluckedString> synthManager {"pluck"};
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
    return 0;
}
