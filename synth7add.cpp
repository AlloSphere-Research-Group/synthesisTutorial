
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


    Pan<> mPan;
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
    EnvFollow<> mEnvFollow;

    // Additional members
    Mesh mMesh;

     AddSyn()
    {
//        set(6.2,155.6,0.01,0.5,0.1,0.1,0.8,0.5,0.001,0.1,0.8,0.6,0.01,0.075,0.9,1,2.001,3,4.00009,5.0002,6,7,8,9);
//        //set(6.2,155.6,0.01,0.5,0.01,0.01,0.8,0.5,0.001,0.1,0.8,0.6,0.01,0.075,0.9,1,2.001,3,4.00009,5.0002,6,7,8,9);
//        mEnvStri.curve(-4); // make segments lines
//        mEnvStri.levels(0,1,1,0);
//        mEnvLow.curve(-4); // make segments lines
//        mEnvLow.levels(0,1,1,0);
//        mEnvUp.curve(-4); // make segments lines
//        mEnvUp.levels(0,1,1,0);
    }

     virtual void init() {

       // Intialize envelopes
         mEnvStri.curve(-4); // make segments lines
         mEnvStri.levels(0,1,1,0);
//         mEnvStri.sustain(2); // Make point 2 sustain until a release is issued
         mEnvLow.curve(-4); // make segments lines
         mEnvLow.levels(0,1,1,0);
//         mEnvLow.sustain(2); // Make point 2 sustain until a release is issued
         mEnvUp.curve(-4); // make segments lines
         mEnvUp.levels(0,1,1,0);
//         mEnvUp.sustain(2); // Make point 2 sustain until a release is issued


       // We have the mesh be a sphere
       addDisc(mMesh, 1.0, 30);

       createInternalTriggerParameter("amp", 0.1, 0.0, 1.0);
       createInternalTriggerParameter("freq", 60, 20, 5000);
       createInternalTriggerParameter("ampStri", 0.1, 0.0, 1.0);
       createInternalTriggerParameter("attackStri", 0.1, 0.01, 3.0);
       createInternalTriggerParameter("releaseStri", 3.0, 0.1, 10.0);
       createInternalTriggerParameter("sustainStri", 0.7, 0.0, 1.0);
       createInternalTriggerParameter("ampLow", 0.1, 0.0, 1.0);
       createInternalTriggerParameter("attackLow", 0.1, 0.01, 3.0);
       createInternalTriggerParameter("releaseLow", 3.0, 0.1, 10.0);
       createInternalTriggerParameter("sustainLow", 0.7, 0.0, 1.0);
       createInternalTriggerParameter("ampUp", 0.1, 0.0, 1.0);
       createInternalTriggerParameter("attackUp", 0.1, 0.01, 3.0);
       createInternalTriggerParameter("releaseUp", 3.0, 0.1, 10.0);
       createInternalTriggerParameter("sustainUp", 0.7, 0.0, 1.0);
       createInternalTriggerParameter("freqStri1", 400, 20, 5000);
       createInternalTriggerParameter("freqStri2", 500, 20, 5000);
       createInternalTriggerParameter("freqStri3", 600, 20, 5000);
       createInternalTriggerParameter("freqLow1", 60, 20, 5000);
       createInternalTriggerParameter("freqLow2", 160, 20, 5000);
       createInternalTriggerParameter("freqUp1", 860, 20, 5000);
       createInternalTriggerParameter("freqUp2", 1060, 20, 5000);
       createInternalTriggerParameter("freqUp3", 2060, 20, 5000);
       createInternalTriggerParameter("freqUp4", 3060, 20, 5000);
       createInternalTriggerParameter("pan", 0.0, -1.0, 1.0);
     }

//    AddSyn& freq(float v){
//        mOscFrq=v;

//        mOsc1.freq(mfreqStri1*mOscFrq);
//        mOsc2.freq(mfreqStri2*mOscFrq);
//        mOsc3.freq(mfreqStri3*mOscFrq);
//        mOsc4.freq(mfreqLow1*mOscFrq);
//        mOsc5.freq(mfreqLow2*mOscFrq);
//        mOsc6.freq(mfreqUp1*mOscFrq);
//        mOsc7.freq(mfreqUp2*mOscFrq);
//        mOsc8.freq(mfreqUp3*mOscFrq);
//        mOsc9.freq(mfreqUp4*mOscFrq);
//        return *this;
//    }

//    AddSyn& freqStri1(float v){ mfreqStri1=v; mOsc1.freq(v*mOscFrq); return *this; }
//    AddSyn& freqStri2(float v){ mfreqStri2=v; mOsc2.freq(v*mOscFrq); return *this; }
//    AddSyn& freqStri3(float v){ mfreqStri3=v; mOsc3.freq(v*mOscFrq); return *this; }
//    AddSyn& freqLow1(float v){ mfreqLow1=v; mOsc4.freq(v*mOscFrq); return *this; }
//    AddSyn& freqLow2(float v){ mfreqLow2=v; mOsc5.freq(v*mOscFrq); return *this; }
//    AddSyn& freqUp1(float v){ mfreqUp1=v; mOsc6.freq(v*mOscFrq); return *this; }
//    AddSyn& freqUp2(float v){ mfreqUp2=v; mOsc7.freq(v*mOscFrq); return *this; }
//    AddSyn& freqUp3(float v){ mfreqUp3=v; mOsc8.freq(v*mOscFrq); return *this; }
//    AddSyn& freqUp4(float v){ mfreqUp4=v; mOsc9.freq(v*mOscFrq); return *this; }
//    AddSyn& freqUp(float v1, float v2, float v3, float v4) {
//        mOsc6.freq(v1*mOscFrq);
//        mOsc7.freq(v2*mOscFrq);
//        mOsc8.freq(v3*mOscFrq);
//        mOsc9.freq(v4*mOscFrq);
//        return *this;
//    }

//    AddSyn& freq(float v1, float v2) {
//        dur(v1);
//        freq(v2);
//        return *this;
//    }

//    AddSyn& amp(float v){ mAmp=v; return *this; }
//    AddSyn& ampStri(float v){ mAmpStri=v; return *this; }
//    AddSyn& attackStri(float v){ mEnvStri.lengths()[0] = v; return *this; }
//    AddSyn& decayStri(float v){ mEnvStri.lengths()[2] = v; return *this; }
//    AddSyn& susStri(float v){ mEnvStri.levels()[2]=v; return *this; }
//    AddSyn& ampLow(float v){ mAmpLow=v; return *this; }
//    AddSyn& attackLow(float v){ mEnvLow.lengths()[0] = v; return *this; }
//    AddSyn& decayLow(float v){ mEnvLow.lengths()[2] = v; return *this; }
//    AddSyn& susLow(float v){ mEnvLow.levels()[2]=v; return *this; }
//    AddSyn& ampUp(float v){ mAmpUp=v; return *this; }
//    AddSyn& attackUp(float v){ mEnvUp.lengths()[0] = v; return *this; }
//    AddSyn& decayUp(float v){ mEnvUp.lengths()[2] = v; return *this; }
//    AddSyn& susUp(float v){ mEnvUp.levels()[2]=v; return *this; }


//    AddSyn& dur(float v){ mDur=v; return *this; }

//    AddSyn& pan(float v){ mPan.pos(v); return *this; }

//    AddSyn& set(
//        float a, float b, float c, float d, float e,float f, float g, float h, float i, float j,
//                float k, float l, float m, float n, float o, float p, float q, float r, float s, float t,
//                float u, float v, float w, float x, float y=0
//    ){
//        return dur(a).freq(b).amp(c).ampStri(d).attackStri(e).decayStri(f).susStri(g).ampLow(h)
//        .attackLow(i).decayLow(j).susLow(k).ampUp(l).attackUp(m).decayUp(n).susUp(o)
//        .freqStri1(p).freqStri2(q).freqStri3(r).freqLow1(s).freqLow2(t).freqUp1(u)
//        .freqUp2(v).freqUp3(w).freqUp4(x).pan(y);
//    }

    //
     virtual void onProcess(AudioIOData& io) override {
       // Parameters will update values once per audio callback
       mOsc.freq(getInternalParameterValue("freq"));
       mOsc1.freq(getInternalParameterValue("freqStri1"));
       mOsc2.freq(getInternalParameterValue("freqStri2"));
       mOsc3.freq(getInternalParameterValue("freqStri3"));
       mOsc4.freq(getInternalParameterValue("freqLow1"));
       mOsc5.freq(getInternalParameterValue("freqLow2"));
       mOsc6.freq(getInternalParameterValue("freqUp1"));
       mOsc7.freq(getInternalParameterValue("freqUp2"));
       mOsc8.freq(getInternalParameterValue("freqUp3"));
       mOsc9.freq(getInternalParameterValue("freqUp4"));
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
//        mEnvStri.totalLength(mDur, 1);
//        mEnvLow.totalLength(mDur, 1);
//        mEnvUp.totalLength(mDur, 1);

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
         mEnvStri.release();
         mEnvLow.release();
         mEnvUp.release();
     }


};

//class Chimes : public AddSyn {
//public:
//    Chimes(): AddSyn() {
//        set (6.2,440,0.1,0.5,0.0001,3.8,0.3,0.4,0.0001,6.0,0.99,0.3,0.0001,6.0,0.9,2,3,4.07,0.56,0.92,1.19,1.7,2.75,3.36);
//    }
//};


//void addOneNote(SynthSequencer &s) {
//    s.add<Chimes>(0.0).freq(6.2,622.2);
//}


//float harmonicSeriesScale[20];

//void initScaleToHarmonicSeries() {
//    for (int i=0;i<20;++i) {
//        harmonicSeriesScale[i] = 100*i;
//    }
//}
//float randomFromHarmonicSeries() {
//    int index = rnd::uni(0,20);
//    // std::cout << "index " << index << " is " << myScale[index] << std::endl;
//    return harmonicSeriesScale[index];
//}


//float halfStepScale[20];
//float halfStepInterval = 1.05946309; // 2^(1/12)
//void initScaleTo12TET(float lowest) {
//    float f = lowest;
//    for (int i=0;i<20;++i) {
//        halfStepScale[i] = f;
//        f *= halfStepInterval;
//    }
//}

//float randomFrom12TET() {
//    int index = rnd::uni(0,20);
//    // std::cout << "index " << index << " is " << myScale[index] << std::endl;
//    return halfStepScale[index];
//}


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

//// Chooses frequencies from a random uniform distribution

//void fillTime(SynthSequencer &s, float from, float to, float minattackStri, float minattackLow, float minattackUp, float maxattackStri, float maxattackLow, float maxattackUp, float minFreq, float maxFreq) {
//    while (from <= to) {
//        float nextAtt = rnd::uni((minattackStri+minattackLow+minattackUp),(maxattackStri+maxattackLow+maxattackUp));
//        s.add<Chimes>(from).attackStri(nextAtt).freq(rnd::uni(minFreq,maxFreq)).amp(0.02);
//        std::cout << "old from " << from << " plus nextnextAtt " << nextAtt << std::endl;
//        from += nextAtt;
//    }
//}

//void fillTimeWith12TET(SynthSequencer &s, float from, float to, float minattackStri, float minattackLow, float minattackUp, float maxattackStri, float maxattackLow, float maxattackUp) {
//    while (from <= to) {
//        float nextAtt = rnd::uni((minattackStri+minattackLow+minattackUp),(maxattackStri+maxattackLow+maxattackUp));
//        float f = randomFrom12TET();
//        s.add<Chimes>(from).attackStri(nextAtt).freq(f).amp(0.02);
//        std::cout << "12 old from " << from << " plus nextAtt " << nextAtt << std::endl;
//        std::cout << "12 old from " << from << " plus nextAtt " << nextAtt << std::endl;
//        from += nextAtt;
//    }
//}


//int main(){
//    // initScaleTo12TET(110);
//    initScaleToMyPitches();
//    initScaleTo12TET(110);

//    SynthSequencer s;

//    s.add<AddSyn>(0).set(6.2,155.6,0.1,0.5,0.0001,3.8,0.3,0.4,0.0001,6.0,0.99,0.3,0.0001,6.0,0.9,2,3,4.07,0.56,0.92,1.19,1.7,2.75,3.36);
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


//    AudioIO io;
//    io.init( s.audioCB, &s,256, 44100.);
//    Domain::master().spu(io.framesPerSecond());
//    io.start();
//    printf("\nPress 'enter' or Ctrl-C to quit...\n");
//    while (io.isRunning()) {
//      auto c = getchar();
//      if (c != -1) {
//        std::cout << c <<std::endl;
//        io.stop();
//      }
//    }
//    return 0;
//}

class MyApp : public App
{
public:

    // GUI manager for SineEnv voices
    // The name provided determines the name of the directory
    // where the presets and sequences are stored
    SynthGUIManager<AddSyn> synthManager {"synth7new"};

    // This function is called right after the window is created
    // It provides a grphics context to initialize ParameterGUI
    // It's also a good place to put things that should
    // happen once at startup.
    virtual void onCreate() override {
        ParameterGUI::initialize();

        // Play example sequence. Comment this line to start from scratch
        synthManager.synthSequencer().playSequence("synth7new.synthSequence");
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
        synthManager.drawSynthControlPanel();
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
