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
#include "al/core/graphics/al_Mesh.hpp"
#include "al/core/graphics/al_Shapes.hpp"
#include "al/util/ui/al_SynthSequencer.hpp"

using namespace gam;
using namespace al;

// This SineEnv class is the same as the one in the original synth1.cpp file
// except for the addition of code for the onProcess(Graphics &g) function
// and the addition of a Mesh (mMesh) that will be drawn
class SineEnv : public SynthVoice {
public:

    SineEnv()
    {
        set (6.5, 60, 0.3, 1, 2);
        mAmpEnv.curve(0); // make segments lines
        mAmpEnv.levels(0,1,1,0);

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
        g.scale(1- mDur, mDur, 1);
        g.color(1, mOsc.freq()/1000, mEnvFollow.value());
        g.draw(mMesh);
        g.popMatrix();
    }

    virtual void onTriggerOn() override {
        mAmpEnv.totalLength(mDur, 1);
        mAmpEnv.reset();
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


// Now we make an app that displays a window
class MyApp : public App
{
public:
    // Instead of using an AudioIO object like before, we use the app's
    // onSound callback and we have the sequencer render audio here
    virtual void onSound(AudioIOData &io) override {
        s.render(io); // Render audio
    }

    // Same case for the graphics. We have the sequencer render graphics
    // here
    virtual void onDraw(Graphics &g) override {
        g.clear();
        s.render(g); // Render graphics
    }

    SynthSequencer s;
};


int main(){

    MyApp app; // Create the app

    SynthSequencer &s = app.s; // For convenience, get a reference to the app's sequencer

    s.setTempo(60);
    // Now we sequence stuff. This is the C++ "note list"
    //s.add( 0).set(2.5,  60, 0.3, .1, .2);
    s.add<SineEnv>( 0  ).set(3.5, 260, 0.3, .011, .2);
    s.add<SineEnv>( 0  ).set(3.5, 510, 0.3, .011, .2);
    s.add<SineEnv>( 3.5).set(3.5, 233, 0.3, .011, .2);
    s.add<SineEnv>( 3.5).set(3.5, 340, 0.3, .011, .2);
    s.add<SineEnv>( 3.5).set(7.5, 710, 0.3, 1, 2);
    //s.add<SineEnv>( 6.5).set(1.5,  60, 0.3, 1, 2);
    s.add<SineEnv>( 6.5).set(1.5,  60, 0.3, .1, .2);
    s.add<SineEnv>( 8.0).set(3.5, 230, 0.3, 1, 2);
    s.add<SineEnv>(11.5).set(7.5, 450, 0.3, 1, 2);
    s.add<SineEnv>(11.5).set(3.0, 240, 0.3, 1, 2);
    s.add<SineEnv>(11.5).set(1.0,  90, 0.3, .1, .2);
    s.add<SineEnv>(12.5).set(2.0, 120, 0.3, .1,.2);
    s.add<SineEnv>(14.5).set(4.5, 380, 0.3, 1, 2);
    s.add<SineEnv>(14.5).set(3.0, 240, 0.3, 1, 2);
    //s.add<SineEnv>(17.5).set(1.5, 110, 0.3, 1, 2);
    s.add<SineEnv>(17.5).set(1.5, 110, 0.3, .1, .2);
    s.add<SineEnv>(19.0).set(8.0, 940, 0.3, 1, 2);
    s.add<SineEnv>(19.0).set(8.0, 780, 0.3, 1, 2);
    s.add<SineEnv>(19.0).set(8.0, 640, 0.3, 1, 2);
    s.add<SineEnv>(27.0).freq(233).amp(0.1);
    s.add<SineEnv>(27.0).freq(234).amp(0.1);
    s.add<SineEnv>(27.0).freq(235).amp(0.1);
    s.add<SineEnv>(27.0).freq(232).amp(0.1);
    s.add<SineEnv>(27.0).freq(231).amp(0.1);
    s.add<SineEnv>(27.0).freq(230).amp(0.1);
    s.add<SineEnv>(31.0).freq(329.6).amp(0.1);
    s.add<SineEnv>(31.0).freq(330.6).amp(0.1);
    s.add<SineEnv>(31.0).freq(331.6).amp(0.1);
    s.add<SineEnv>(31.0).freq(334.6).amp(0.1);
    s.add<SineEnv>(31.0).freq(335.6).amp(0.1);
    s.add<SineEnv>(31.0).freq(337.6).amp(0.1);
    s.add<SineEnv>(35.0).freq(233).amp(0.1);
    s.add<SineEnv>(35.0).freq(234).amp(0.1);
    s.add<SineEnv>(35.0).freq(235).amp(0.1);
    s.add<SineEnv>(35.0).freq(232).amp(0.1);
    s.add<SineEnv>(35.0).freq(231).amp(0.1);
    s.add<SineEnv>(35.0).freq(230).amp(0.1);
    s.add<SineEnv>(39.0).freq(440).amp(0.1);
    s.add<SineEnv>(39.0).freq(441).amp(0.1);
    s.add<SineEnv>(39.0).freq(442).amp(0.1);
    s.add<SineEnv>(39.0).freq(443).amp(0.1);
    s.add<SineEnv>(39.0).freq(445).amp(0.1);
    s.add<SineEnv>(39.0).freq(446).amp(0.1);
    s.add<SineEnv>(45.0).set(8.0, 940, 0.1, 1, 2);
    s.add<SineEnv>(45.0).set(8.0, 941, 0.1, 1, 2);
    s.add<SineEnv>(45.0).set(8.0, 942, 0.1, 1, 2);
    s.add<SineEnv>(47.0).set(8.0, 780, 0.1, 1, 2);
    s.add<SineEnv>(47.0).set(8.0, 781, 0.1, 1, 2);
    s.add<SineEnv>(47.0).set(8.0, 782, 0.1, 1, 2);
    s.add<SineEnv>(49.0).set(8.0, 640, 0.1, 1, 2);
    s.add<SineEnv>(49.0).set(8.0, 641, 0.1, 1, 2);
    s.add<SineEnv>(49.0).set(8.0, 642, 0.1, 1, 2);
    s.add<SineEnv>(51.5).set(7.5, 450, 0.3, 1, 2);
    s.add<SineEnv>(51.5).set(3.0, 240, 0.3, 1, 2);
    //s.add<SineEnv>(51.5).set(1.0,  90, 0.3, 1, 2);
    //s.add<SineEnv>(52.5).set(2.0, 120, 0.3, 1, 2);
    s.add<SineEnv>(51.5).set(1.0,  90, 0.3, .1, .2);
    s.add<SineEnv>(52.5).set(2.0, 120, 0.3, .1, .2);
    s.add<SineEnv>(54.5).set(4.5, 380, 0.3, 1, 2);
    s.add<SineEnv>(54.5).set(3.0, 240, 0.3, 1, 2);
    //s.add<SineEnv>(57.5).set(1.5, 110, 0.3, 1, 2);
    s.add<SineEnv>(57.5).set(1.5, 110, 0.3, .1, .2);
    s.add<SineEnv>( 68  ).set(6.5, 260, 0.3, 1, 2);
    s.add<SineEnv>( 68  ).set(3.5, 510, 0.3, 1, 2);
    s.add<SineEnv>( 65.5).set(3.0, 233, 0.3, 1, 2);
    s.add<SineEnv>( 65.5).set(4.5, 340, 0.3, 1, 2);
    s.add<SineEnv>( 65.5).set(7.5, 710, 0.3, 1, 2);
    //s.add<SineEnv>( 62.5).set(1.5,  60, 0.3, 1, 2);
    s.add<SineEnv>( 62.5).set(1.5,  60, 0.3, .1, .2);
    s.add<SineEnv>( 60.0).set(3.5, 230, 0.3, 1, 2);

    // Now we start the app
    app.initAudio(44100., 256, 2, 0);
    Domain::master().spu(app.audioIO().framesPerSecond());
    app.start();

}
