/*    Gamma - Generic processing library
    See COPYRIGHT file for authors and license information

    Example:
    Description:
*/


#include <cstdio>               // for printing to stdout
#define GAMMA_H_INC_ALL         // define this to include all header files
#define GAMMA_H_NO_IO           // define this to avoid bringing AudioIO from Gamma

#include "Gamma/Gamma.h"

#include "allocore/io/al_AudioIO.hpp"
#include "allocore/ui/al_SynthSequencer.hpp"

using namespace gam;
using namespace al;


class FM : public SynthVoice {
public:
    FM()
    :  mAmp(1), mDur(2),
        chrA1(0.31, 0.002, 0.20111, -0.7, 0.9),
        chrA2(0.22, 0.002, 0.10151, -0.7, 0.9),
        chrA3(0.13, 0.002, 0.05131, -0.7, 0.9),
        mCarFrq(440), mCarMul(1), mModMul(1), mModAmt(50)
    {
        set( 5, 262, 0.5, 0.1,0.1, 0.75, 0.01,7,5, 1,1.0007,3.5, 7.5, 0.5, 0.0075, 0);

    }

    FM& freq(float v){ mCarFrq=v; return *this; }
    FM& amp(float v){ mAmp=v; return *this; }
    FM& attack(float v){
        mAmpEnv.lengths()[0] = v;
        mModEnv.lengths()[0] = v;
        return *this;
    }
    FM& decay(float v){
        mAmpEnv.lengths()[2] = v;
        mModEnv.lengths()[2] = v;
        return *this;
    }

    FM& sus(float v){
        mAmpEnv.levels()[2] = 1;
        return *this;
    }

    FM& dur(float v){ mDur=v; return *this; }

    FM& pan(float v){ mPan.pos(v); return *this; }

    FM& carMul(float v){ mCarMul=v; return *this; }
    FM& modMul(float v){ mModMul=v; return *this; }
    FM& modAmt(float v){ mModAmt=v; return *this; }

    FM& idx1(float v){ mModEnv.levels()[0]=v; return *this; }
    FM& idx2(float v){ mModEnv.levels()[1]=v; return *this; }
    FM& idx3(float v){ mModEnv.levels()[2]=v; return *this; }
    FM& vibRate1(float v){ mVibEnv.levels(v,mVibEnv.levels()[1],v); return *this; }
    FM& vibRate2(float v){ mVibEnv.levels()[1]=v; return *this; }
    FM& vibDepth(float v){ mVibDepth=v; return *this; }
    FM& vibRise(float v){ mVibRise=v; return *this; }

    FM& set(float a, float b, float c, float d, float e, float f,
            float g, float h, float i, float j, float k, float l, float m, float n, float o, float p){
        return
            dur(a).freq(b).amp(c).attack(d).decay(e).sus(f)
        .idx1(g).idx2(h).idx3(i)
        .carMul(j).modMul(k).vibRate1(l).vibRate2(m).vibRise(n).vibDepth(o).pan(p);
    }

    //
    virtual void onProcess(AudioIOData& io) override {
        float modFreq = mCarFrq * mModMul;
        while(io()){
            mVib.freq(mVibEnv());
            car.freq((mCarFrq + mVib()*mVibDepth*mCarFrq) *mCarMul + mod()*mModEnv()*modFreq);
            float s = car() * mAmpEnv() * mAmp;
            float s1 = chrA3(chrA2(chrA1(s)));
            float s2;
            mEnvFollow(s1);
            mPan(s1, s1,s2);
            io.out(0) += s1;
            io.out(1) += s2;
        }
        if(mAmpEnv.done() && (mEnvFollow.value() < 0.001f)) free();
    }

    virtual void onTriggerOn() override {

        float modFreq = mCarFrq * mModMul;
        mod.freq(modFreq);

        mAmpEnv.totalLength(mDur, 1);
        mModEnv.lengths()[1] = mAmpEnv.lengths()[1];
        mVibEnv.lengths()[0] = mDur * (1-mVibRise);
        mVibEnv.lengths()[1] = mDur * mVibRise;

        mAmpEnv.reset();
        mModEnv.reset();
        mVibEnv.reset();
    }

protected:
    // general synth parameters
    //float mPitch; // implicit
    float mAmp;
    float mDur;
    Pan<> mPan;

    // specific parameters
    float mCarFrq;        // carrier frequency
    float mCarMul;        // carrier frequency multiplier
    float mModMul;        // modulator frequency multiplier
    float mModAmt;        // frequency modulation amount
    float mVibFrq;
    float mVibDepth;
    float mVibRise;

    Sine<> car, mod, mVib;    // carrier, modulator sine oscillators
    Env<3> mAmpEnv;
    Env<3> mModEnv;
    Env<2> mVibEnv;
    Chorus<> chrA1, chrA2, chrA3;
    EnvFollow<> mEnvFollow;
};


int main(){

    SynthSequencer s;
    s.add<FM>( 0).set( 5, 262, 0.5, 0.1,1.0, 0.75, 0.01,7,5, 1,1.0007,3.5, 7.5, 0.5, 0.0075, 0);
    s.add<FM>( 5).set( 5, 262, 0.5, 0.1,0.1, 0.75, 0.01,4,4, 3,2.0007,3.5, 5.8, 0.5, 0.0075,0);
    s.add<FM>(10).set( 5, 262, 0.5, 0.2,0.1, 0.75, 2.00,2,2, 3,1.0007,9.8, 3.5, 0.5, 0.0075, 0);
    s.add<FM>(15).set( 5, 139, 0.5, 0.2,0.1, 0.75, 0.01,1.5,1.5, 5,1.0007,3.5, 7.0, 1.0, 0.0075, 0);
    s.add<FM>(20).set(10, 100, 0.5, 0.001,9.90, 0.8, 7.0, 7.0, 7.0, 1, 1.4, 3.5, 7.5, 0.5, 0.0075, 0);
    s.add<FM>(30).set(0.3,100, 0.5, 0.001,0.25, 0.8, 5,5,5, 1, 1.48, 3.5, 7.5, 0.5, 0.0075, 0);


    AudioIO io(256, 44100., s.audioCB, &s);
    Domain::master().spu(io.framesPerSecond());
    io.start();
    printf("\nPress 'enter' to quit...\n"); getchar();
}
