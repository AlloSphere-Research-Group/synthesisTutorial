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
#include "al/util/ui/al_SynthSequencer.hpp"
#include "al/util/imgui/al_Imgui.hpp"

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

    SineEnv& pan(float v){ mPan.pos(v); return *this; }

    SineEnv& set(
        float a, float b, float c, float d, float e, float f=0
    ){
        return dur(a).freq(b).amp(c).attack(d).decay(e).pan(f);
    }

    //
    virtual void onProcess(AudioIOData& io) override {
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
    }

    virtual void onSound(AudioIOData &io) override {
        pSynth.render(io); // Render audio
    }

    virtual void onDraw(Graphics &g) override {
        beginIMGUI();
        g.clear();
        pSynth.render(g);
        ImGui::SetNextWindowSize(ImVec2(400, 120), ImGuiCond_FirstUseEver);
        ImGui::Begin("Synth1"); // Begin a window called "Synth1"
        // Create 3 sliders
        ImGui::SliderFloat("Amplitude", &mAmplitudeSliderValue, 0.0f, 1.0f);
        ImGui::SliderFloat("Pan", &mPanSliderValue, -1.0f, 1.0f);
        ImGui::SliderFloat("Attack", &mAttackSliderValue, 0.0f, 1.5f);
        ImGui::End(); // End the window
        endIMGUI();
    }

    virtual void onKeyDown(Keyboard const& k) override {
        int midiNote = asciiToMIDI(k.key());
        if (midiNote > 0) {
            float frequency = ::pow(2., (midiNote - 69.)/12.) * 440.;
            SineEnv *voice = pSynth.getVoice<SineEnv>();
            voice->freq(frequency);
            // We use the values from sliders to configure the triggered voice
            voice->amp(mAmplitudeSliderValue);
            voice->pan(mPanSliderValue);
            voice->attack(mAttackSliderValue);
            pSynth.triggerOn(voice, 0, midiNote);
        }
    }
    virtual void onKeyUp(Keyboard const& k) override {
        int midiNote = asciiToMIDI(k.key());
        if (midiNote > 0) {
            pSynth.triggerOff(midiNote);
        }
    }

    void onExit() override {
        shutdownIMGUI();
    }

    PolySynth pSynth;

    float mAmplitudeSliderValue {0.5};
    float mPanSliderValue {0.0};
    float mAttackSliderValue {0.0};
};


int main(){    // Create app instance
    MyApp app;

    // Pre-allocate voice to avoid real-time allocation
    // Real time allocation is usually not a problem but can create glitches
    // occasionally
    app.pSynth.allocatePolyphony<SineEnv>(16);

    // Set output gain for PolySynth to avoid clipping
    app.pSynth.gain(0.1);

    app.navControl().active(false); // Disable navigation via keyboard, since we will be using keyboard for note triggering

    // Set up audio
    app.initAudio(44100., 256, 2, 0);
    Domain::master().spu(app.audioIO().framesPerSecond());

    app.start();
}
