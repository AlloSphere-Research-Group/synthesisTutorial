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
#include "al/util/scene/al_PolySynth.hpp"
#include "al/util/scene/al_SynthSequencer.hpp"
#include "al/util/ui/al_ControlGUI.hpp"

//using namespace gam;
using namespace al;

// This is the same SineEnv class defined in graphics/synth1.cpp
// It inclludes drawing code
class SineEnv : public SynthVoice {
public:

    // Voice parameters
    Parameter amplitude {"amplitude", "", 0.3, "", 0.0, 1.0};
    Parameter frequency {"frequency", "", 60, "", 20, 1000};
    Parameter attackTime {"attackTime", "", 1.0, "", 0.01, 3.0};
    Parameter releaseTime {"releaseTime", "", 3.0, "", 0.1, 10.0};
    Parameter pan {"pan", "", 0.0, "", -1.0, 1.0};

    // Unit generators
    gam::Pan<> mPan;
    gam::Sine<> mOsc;
    gam::Env<3> mAmpEnv;
    gam::EnvFollow<> mEnvFollow;

    // Additional members
    Mesh mMesh;

    // Initialize voice. This function will nly be called once per voice
    virtual void init() {

        // Intialize envelope
        mAmpEnv.curve(0); // make segments lines
        mAmpEnv.levels(0,1,1,0);
        mAmpEnv.sustainPoint(2); // Make point 2 sustain until a release is issued

        // We have the mesh be a sphere
        addDisc(mMesh, 1.0, 30);

        // Register the parameters with the SynthVoice.
        // These are the parameters that will be shown in the GUIs
        // And the parameters that will be stored in sequences
        registerTriggerParameters(amplitude, frequency,
                                  attackTime, releaseTime, pan);

        // Connect the parameters to the unit generators.
        // We register a function to be called whenever the parameter changes
        // Only needed when the parameter's value is not used directly,
        // but rather sets some internal state of a unit generator
        frequency.registerChangeCallback([&](float value) {mOsc.freq(value);});
        attackTime.registerChangeCallback([&](float value) {mAmpEnv.lengths()[0] = value;});
        releaseTime.registerChangeCallback([&](float value) {mAmpEnv.lengths()[2] = value;});
        pan.registerChangeCallback([&](float value) {mPan.pos(value);});
    }

    //
    virtual void onProcess(AudioIOData& io) override {
        while(io()){
            float s1 = mOsc() * mAmpEnv() * amplitude;
            float s2;
            mEnvFollow(s1);
            mPan(s1, s1,s2);
            io.out(0) += s1;
            io.out(1) += s2;
        }
        // We need to let the synth know that this voice is done
        // by calling the free(). This takes the voice out of the
        // rendering chain
        if(mAmpEnv.done() && (mEnvFollow.value() < 0.001f)) free();
    }

    virtual void onProcess(Graphics &g) {
        g.pushMatrix();
        g.translate(frequency/200 - 3,  amplitude, -8);
        g.scale(1- amplitude, amplitude, 1);
        g.color(mEnvFollow.value(), mOsc.freq()/1000, mEnvFollow.value()* 10, 0.4);
        g.draw(mMesh);
        g.popMatrix();
    }

    virtual void onTriggerOn() override {
        mAmpEnv.reset();
    }

    virtual void onTriggerOff() override {
        mAmpEnv.release();
    }
};


// We make an app.
class MyApp : public App
{
public:

    virtual void onCreate() override {
        ParameterGUI::initialize();

        // Play example sequence. Comment this line to start from scratch
        synthManager.synthSequencer().playSequence("synth1.synthSequence");
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
        synthManager.drawSynthControlPanel();
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
              // Most configuration will come from the gui, except for the
              // frequency that will be set according to the key pressed
              // So we need to set it manually through synthManager.voice()
              // This voice represents the settings in the gui.
              synthManager.voice()->frequency = ::pow(2.f, (midiNote - 69.f)/12.f) * 432.f;
              // Get a free voice from the synth. This is the voice that will be
              // inserted in the rendering chain
              SineEnv *voice = synthManager.synth().getVoice<SineEnv>();
              // Then we configure the free voice.
              // After setting any values manually, we will set the
              // parameters from the gui using the configureVoice() function
              synthManager.configureVoiceFromGui(voice);
              // Now that the note is configured, we insert it in the synth chain
              // We set the id to be midiNote to be able to turn it off later
              synthManager.synth().triggerOn(voice, 0, midiNote);
            }
        }
    }

    virtual void onKeyUp(Keyboard const& k) override {
        int midiNote = asciiToMIDI(k.key());
        if (midiNote > 0) {
            synthManager.synth().triggerOff(midiNote);
        }
    }

    void onExit() override {
        ParameterGUI::cleanup();
    }

    // GUI manager for SineEnv voices in pSynth
    // The name provided determines the name of the directory
    // where the presets and sequences are stored
    SynthGUIManager<SineEnv> synthManager {"synth1"};

    float mAmplitudeSliderValue {0.5};
    float mPanSliderValue {0.0};
    float mAttackSliderValue {0.0};
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
