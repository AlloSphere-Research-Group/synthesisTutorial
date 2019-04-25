
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

// tables for oscillator
gam::ArrayPow2<float>
    tbSaw(2048), tbSqr(2048), tbImp(2048), tbSin(2048), tbPls(2048),
    tb__1(2048), tb__2(2048), tb__3(2048), tb__4(2048);

class Vib : public SynthVoice {
public:

    // Unit generators
    gam::Pan<> mPan;
    gam::Osc<> mOsc;
    gam::Sine<> mVib;
    gam::ADSR<> mAmpEnv;
    gam::ADSR<> mVibEnv;
    gam::EnvFollow<> mEnvFollow;
    
    float vibValue;

    // Additional members
    Mesh mMesh;

    void init() override {
        mAmpEnv.curve(0); // linear segments
        mAmpEnv.levels(0,1,1,0);
        mVibEnv.curve(0);

        // We have the mesh be a sphere
        addDisc(mMesh, 1.0, 30);

        createInternalTriggerParameter("amplitude", 0.1, 0.0, 1.0);
        createInternalTriggerParameter("frequency", 60, 20, 5000);
        createInternalTriggerParameter("attackTime", 0.1, 0.01, 3.0);
        createInternalTriggerParameter("releaseTime", 3.0, 0.1, 10.0);
        createInternalTriggerParameter("curve", 4.0, -10.0, 10.0);
        createInternalTriggerParameter("pan", 0.0, -1.0, 1.0);
        createInternalTriggerParameter("table", 0, 0, 8);
        createInternalTriggerParameter("vibRate1", 3.5, 0.2, 20);
        createInternalTriggerParameter("vibRate2", 5.8, 0.2, 20);
        createInternalTriggerParameter("vibRise", 0.5, 0.1, 2);
        createInternalTriggerParameter("vibDepth", 0.005, 0.0, 0.3);
    }

    virtual void onProcess(AudioIOData& io) override {

        float oscFreq = getInternalParameterValue("frequency");
        float amp = getInternalParameterValue("amplitude");
        float vibDepth = getInternalParameterValue("vibDepth");
        while(io()){
            mVib.freq(mVibEnv());
            vibValue = mVib();
            mOsc.freq(oscFreq + vibValue*vibDepth*oscFreq);

            float s1 = mOsc() * mAmpEnv() * amp;
            float s2;
            mEnvFollow(s1);
            mPan(s1, s1,s2);
            io.out(0) += s1;
            io.out(1) += s2;
        }
        //if(mAmpEnv.done()) free();
        if(mAmpEnv.done() && (mEnvFollow.value() < 0.001)) free();
    }

virtual void onProcess(Graphics &g) {
        float frequency = getInternalParameterValue("frequency");
        float amplitude = getInternalParameterValue("amplitude");
        g.pushMatrix();
        g.translate(amplitude,  amplitude, -4);
        float scaling = vibValue + getInternalParameterValue("vibDepth");
        g.scale(scaling * frequency/200, scaling * frequency/400, scaling* 1);
        g.color(mEnvFollow.value(), frequency/1000, mEnvFollow.value()* 10, 0.4);
        g.draw(mMesh);
        g.popMatrix();
    }
    
    virtual void onTriggerOn() override {
//        mAmpEnv.totalLength(mDur, 1);
        updateFromParameters();

        mAmpEnv.reset();
        mVibEnv.reset();
        // Map table number to table in memory
        switch (int(getInternalParameterValue("table"))) {
        case 0: mOsc.source(tbSaw); break;
        case 1: mOsc.source(tbSqr); break;
        case 2: mOsc.source(tbImp); break;
        case 3: mOsc.source(tbSin); break;
        case 4: mOsc.source(tbPls); break;
        case 5: mOsc.source(tb__1); break;
        case 6: mOsc.source(tb__2); break;
        case 7: mOsc.source(tb__3); break;
        case 8: mOsc.source(tb__4); break;
        }
    }

    virtual void onTriggerOff() override {
        mAmpEnv.triggerRelease();
        mVibEnv.triggerRelease();
    }

    void updateFromParameters() {
        mOsc.freq(getInternalParameterValue("frequency"));
        mAmpEnv.attack(getInternalParameterValue("attackTime"));
        mAmpEnv.decay(getInternalParameterValue("attackTime"));
        mAmpEnv.release(getInternalParameterValue("releaseTime"));
        mAmpEnv.curve(getInternalParameterValue("curve"));
        mPan.pos(getInternalParameterValue("pan"));
        mVibEnv.levels(getInternalParameterValue("vibRate1"),
                       getInternalParameterValue("vibRate2"),
                       getInternalParameterValue("vibRate2"),
                       getInternalParameterValue("vibRate1"));
        mVibEnv.lengths()[0] = getInternalParameterValue("vibRise");
        mVibEnv.lengths()[1] = getInternalParameterValue("vibRise");
        mVibEnv.lengths()[3] = getInternalParameterValue("vibRise");
    }

};

class MyApp : public App
{
public:

    virtual void onInit( ) override {
        gam::addSinesPow<1>(tbSaw, 9,1);
        gam::addSinesPow<1>(tbSqr, 9,2);
        gam::addSinesPow<0>(tbImp, 9,1);
        gam::addSine(tbSin);

        {    float A[] = {1,1,1,1,0.7,0.5,0.3,0.1};
            gam::addSines(tbPls, A,8);
        }

        {    float A[] = {1, 0.4, 0.65, 0.3, 0.18, 0.08};
            float C[] = {1,4,7,11,15,18};
            gam::addSines(tb__1, A,C,6);
        }

        // inharmonic partials
        {    float A[] = {0.5,0.8,0.7,1,0.3,0.4,0.2,0.12};
            float C[] = {3,4,7,8,11,12,15,16};
            gam::addSines(tb__2, A,C,8);
        }

        // inharmonic partials
        {    float A[] = {1, 0.7, 0.45, 0.3, 0.15, 0.08};
            float C[] = {10, 27, 54, 81, 108, 135};
            gam::addSines(tb__3, A,C,6);
        }

        // harmonics 20-27
        {    float A[] = {0.2, 0.4, 0.6, 1, 0.7, 0.5, 0.3, 0.1};
            gam::addSines(tb__4, A,8, 20);
        }
    }

    virtual void onCreate() override {
        ParameterGUI::initialize();

        // Play example sequence. Comment this line to start from scratch
        synthManager.synthSequencer().playSequence("synth2.synthSequence");
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

    SynthGUIManager<Vib> synthManager {"synth3"};
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
