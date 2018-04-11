/*    Gamma - Generic processing library
    See COPYRIGHT file for authors and license information

    Example:
    Description:
*/

#include <cstdio>               // for printing to stdout
#define GAMMA_H_INC_ALL         // define this to include all header files
#define GAMMA_H_NO_IO           // define this to avoid bringing AudioIO from Gamma

#include "Gamma/Gamma.h"

#include "al/core/app/al_App.hpp"
#include "al/core/graphics/al_Shapes.hpp"

#include "al/util/ui/al_Parameter.hpp"
#include "al/util/ui/al_Preset.hpp"
#include "al/util/ui/al_SynthRecorder.hpp"
#include "al/util/ui/al_SynthSequencer.hpp"
#include "al/util/ui/al_SynthGUI.hpp"
//#include "al/util/imgui/al_Imgui.hpp"

using namespace gam;
using namespace al;

// This is the same SineEnv class defined in graphics/synth1.cpp
// It inclludes drawing code
class SineEnv : public SynthVoice {
public:

    SineEnv()
    {
        set (6.5, 60, 0.3, 1, 2);
        mAmpEnv.curve(0); // make segments lines
        mAmpEnv.levels(0,1,1,0);
        mAmpEnv.sustainPoint(2); // Make point 2 sustain until a release is issued

        // We have the mesh be a sphere
        addSphere(mMesh, 0.2, 30, 30);
    }

    // Note parameters
    SineEnv& freq(float v){ mOsc.freq(v); return *this; }
    SineEnv& amp(float v){ mAmp=v; return *this; }
    SineEnv& attack(float v){
        mAmpEnv.lengths()[0] = v;
        return *this;
    }
    SineEnv& decay(float v){
        mAmpEnv.lengths()[2] = v;
        return *this;
    }

    SineEnv& dur(float v){ mDur=v; return *this; }

    SineEnv& pan(float v){ mPanPos = v; return *this; }

    SineEnv& set(
        float a, float b, float c, float d, float e, float f=0
    ){
        return dur(a).freq(b).amp(c).attack(d).decay(e).pan(f);
    }

    virtual bool setParamFields(float *pFields, int numFields) override {
        if (numFields != 6) {
            return false;
        }
        set(pFields[0], pFields[1], pFields[2], pFields[3], pFields[4], pFields[5]);
        return true;
    }

    virtual int getParamFields(float *pFields) override {
        pFields[0] = mDur;
        pFields[1] = mOsc.freq();
        pFields[2] = mAmp;
        pFields[3] = mAmpEnv.lengths()[0];
        pFields[4] = mAmpEnv.lengths()[2];
        pFields[5] = mPanPos;
        return 6;
    }

    //
    virtual void onProcess(AudioIOData& io) override {
        mPan.pos(mPanPos);
        while(io()){
            float s1 = mOsc() * mAmpEnv() * mAmp;
            float s2;
            mEnvFollow(s1);
            mPan(s1, s1,s2);
            io.out(0) += s1;
            io.out(1) += s2;
        }
        //if(mAmpEnv.done()) free();
        if(mAmpEnv.done() && (mEnvFollow.value() < 0.001f)) free();
    }

    virtual void onProcess(Graphics &g) {
        g.pushMatrix();
        g.blendOn();
        g.translate(mOsc.freq()/200 - 3,  pow(mAmp, 0.3), -8);
        g.scale(1- mAmp, mAmp, 1);
        g.color(1, mOsc.freq()/1000, mEnvFollow.value()* 10);
        g.draw(mMesh);
        g.popMatrix();
    }

    virtual void onTriggerOn() override {
        mAmpEnv.totalLength(mDur, 1);
        mAmpEnv.reset();
    }

    virtual void onTriggerOff() override {
        mAmpEnv.release();
    }

protected:

    float mAmp;
    float mDur;
    float mPanPos;
    Pan<> mPan;
    Sine<> mOsc;
    Env<3> mAmpEnv;
    EnvFollow<> mEnvFollow;

    Mesh mMesh;
};

// We again make an app.
// We will use PolySynth to handle voice triggering and allocation
// There is no need for a SynthSequencer here as this is designed for real time

// We will make the GUI through ImGUI. To use it:
// 1. Place setup and cleanup functions in onCreate() and onExit()
// 2. Place beginIMGUI() and endIMGUI() on the edges of the onDraw() call
// 3. Place IMGUI calls inside the onDraw function. For widgets that have
//    values, you will need to create variables for them (e.g.
//    mAmplitudeSliderValue, mPanSliderValue). These will contain the values
//    for the widget. You can use these variables whenever you need the value
//    from the widget
class MyApp : public App
{
public:

    virtual void onCreate() override {
        initIMGUI();
        mGui << mAmplitude << mPan << mAttack << mDecay;
//        mParamServer << mAmplitude << mPan << mAttack << mDecay;
        mPresetHandler << mAmplitude << mPan << mAttack << mDecay;

        mGui << mPresetHandler << recorder << mSequencer;

        parameterMIDI.connectControl(mAttack, 1, 1);
        parameterMIDI.connectControl(mAmplitude, 7, 1);
        parameterMIDI.connectControl(mPan, 10, 1);

        // Register polysynth with recorder
        // This will pass all events for the polysynth to the recorder
        recorder << mSequencer.synth();
    }

    virtual void onSound(AudioIOData &io) override {
        mSequencer.render(io); // Render audio
    }

    virtual void onDraw(Graphics &g) override {
        beginIMGUI();
        g.clear();
        mSequencer.render(g);
        mGui.onDraw(g);
        endIMGUI();
//        mSequencer.synth().print();
    }

    virtual void onKeyDown(Keyboard const& k) override {
        int midiNote = asciiToMIDI(k.key());
        if (midiNote > 0) {
            float frequency = ::pow(2., (midiNote - 69.)/12.) * 440.;
            SineEnv *voice = mSequencer.synth().getVoice<SineEnv>();
            voice->freq(frequency);
            // We use the values from the parameters to configure the triggered voice
            voice->amp(mAmplitude.get());
            voice->pan(mPan.get());
            voice->attack(mAttack.get());
            voice->decay(mDecay.get());
            mSequencer.synth().triggerOn(voice, 0, midiNote);
        }
    }
    virtual void onKeyUp(Keyboard const& k) override {
        int midiNote = asciiToMIDI(k.key());
        if (midiNote > 0) {
            mSequencer.synth().triggerOff(midiNote);
        }
    }

    void onExit() override {
        shutdownIMGUI();
    }

    static void onMessage(const MIDIMessage& m) {
        std::cout << "got message" << std::endl;
    }

    SynthSequencer mSequencer;

    // The voice parameters
    Parameter mAmplitude {"Amplitude", "", 0.5, "", 0.0f, 1.0f};
    Parameter mPan {"Pan", "", 0.0, "", -1.0f, 1.0f};
    Parameter mAttack {"Attack", "Synth1", 0.2, "", 0.0f, 1.5f};
    Parameter mDecay {"Decay", "Synth1", 1.0, "", 0.0f, 2.5f};

    SynthGUI mGui; // To display/interact with widgets in graphics window
    PresetHandler mPresetHandler; // To store/recall parameter values as presets

    ParameterMIDI parameterMIDI {0}; // Pass device number in constructor
    SynthRecorder recorder {SynthRecorder::SEQUENCER_FORMAT};
};


int main(){    // Create app instance
    MyApp app;

    // Pre-allocate voice to avoid real-time allocation
    // Real time allocation is usually not a problem but can create glitches
    // occasionally
    app.mSequencer.synth().allocatePolyphony<SineEnv>(8);

    // Register the SineEnv class in the PolySynth
    // This allows instatiation of voices from the text sequence
    // when needed.
    app.mSequencer.synth().registerSynthClass<SineEnv>("SineEnv");

    // Set output gain for PolySynth to avoid clipping
    app.mSequencer.synth().gain(0.1);

    app.navControl().active(false); // Disable navigation via keyboard, since we will be using keyboard for note triggering

    // Set up audio
    app.initAudio(44100., 256, 2, 0);
    Domain::master().spu(app.audioIO().framesPerSecond());

    app.start();
}
