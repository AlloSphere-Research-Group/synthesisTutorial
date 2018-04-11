/*    Gamma - Generic processing library
    See COPYRIGHT file for authors and license information

    Example:
    Description:
*/

#include <cstdio>               // for printing to stdout
#define GAMMA_H_INC_ALL         // define this to include all header files
#define GAMMA_H_NO_IO           // define this to avoid bringing AudioIO from Gamma

#include "Gamma/Gamma.h"

#include "al/core/io/al_AudioIO.hpp"
#include "al/util/ui/al_SynthSequencer.hpp"

using namespace gam;
using namespace al;

class SineEnv : public SynthVoice {
public:

    SineEnv(double dt=0)
    {
        set (6.5, 60, 0.3, 1, 2);
        mAmpEnv.curve(0); // make segments lines
        mAmpEnv.levels(0,1,1,0);

    // Could make additional notelist-style parameters for these; for now just setting here:
    mNoiseRate = 8.0;
    mNoiseDepth = 0.0075;
    }

    SineEnv& freq(float v){ mOscFrq = v; return *this; }
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
    void onProcess(AudioIOData& io){

        while(io()){
        mOsc.freq(mOscFrq + mSeg(mNoise)*mNoiseDepth*mOscFrq);
      //mOsc.freq(mOscFrq * (1.0 + (mNoiseDepth * mSeg(mNoise))));
            float s1 = mOsc() * mAmpEnv() * mAmp;
      float s2;
            mEnvFollow(s1);
            mPan(s1, s1,s2);
            io.out(0) += s1;
            io.out(1) += s2;
        }
        //if(mAmpEnv.done()) free();
        if(mAmpEnv.done() && (mEnvFollow.value() < 0.001)) free();
    }

    virtual void onTriggerOn() {
        mAmpEnv.totalLength(mDur, 1);
        mSeg.freq(mNoiseRate);
        mAmpEnv.reset();
    }

protected:
    float mAmp;
    float mDur;
    float mOscFrq;
  float mNoiseRate;   // The number of new random numbers generated each second
  float mNoiseDepth;  // Amplitude of noise
    Pan<> mPan;
    Sine<> mOsc;
    Env<3> mAmpEnv;
    EnvFollow<> mEnvFollow;
  NoiseWhite<> mNoise;
  Seg<float, iplSeq::Linear> mSeg;
};



int main(){

    SynthSequencer s;
    //s.add<SineEnv>( 0).set(2.5,  60, 0.3, .1, .2);
    s.add<SineEnv>( 0  ).set(10.0, 220, 0.3, .011, .2);
    //s.add<SineEnv>( 0  ).set(3.5, 510, 0.3, .011, .2);
    //s.add<SineEnv>( 3.5).set(3.5, 233, 0.3, .011, .2);
    //s.add<SineEnv>( 3.5).set(3.5, 340, 0.3, .011, .2);
    //s.add<SineEnv>( 3.5).set(7.5, 710, 0.3, 1, 2);
    //s.add<SineEnv>( 6.5).set(1.5,  60, 0.3, 1, 2);

    AudioIO io;
    io.initWithDefaults(s.audioCB, &s, true, false);
    Domain::master().spu(io.framesPerSecond());
    io.start();
    printf("\nPress 'enter' to quit...\n"); getchar();
}
