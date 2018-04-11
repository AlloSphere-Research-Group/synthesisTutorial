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


class DBCFM : public SynthVoice {
public:

    DBCFM()
        : mAmp(1), mAmp2(1), mDur(2),
          mCarFrq(440), mCarMul(1), mModMul(1), mModAmt(50) , mCarMul2(1), mModMul2(1), mModAmt2(50)
    {
        set( 10, 262, 0.5, 0.5, 0.1,5.1, 5.1,0.1, 0.75,0.75, 0.01,7,5, 1,1.0007,1,1.414, 0);
    }

    DBCFM& freq(float v){ mCarFrq=v; return *this; }
    DBCFM& amp(float v){ mAmp=v; return *this; }
    DBCFM& amp2(float v){ mAmp2=v; return *this; }

    DBCFM& attack(float v){
        mAmpEnv.lengths()[0] = v;
        mModEnv.lengths()[0] = v;
        return *this;
    }
    DBCFM& decay(float v){
        mAmpEnv.lengths()[2] = v;
        mModEnv.lengths()[2] = v;
        return *this;
    }
    DBCFM& attack2(float v){
        mAmpEnv2.lengths()[0] = v;
        mModEnv.lengths()[0] = v;
        return *this;
    }
    DBCFM& decay2(float v){
        mAmpEnv2.lengths()[2] = v;
        mModEnv.lengths()[2] = v;
        return *this;
    }

    DBCFM& sus(float v){
        mAmpEnv.levels()[2] = 1;
        return *this;
    }

    DBCFM& sus2(float v){
        mAmpEnv2.levels()[2] = 1;
        return *this;
    }
    DBCFM& dur(float v){ mDur=v; return *this; }

    DBCFM& pan(float v){ mPan.pos(v); return *this; }

    DBCFM& carMul(float v){ mCarMul=v; return *this; }
    DBCFM& modMul(float v){ mModMul=v; return *this; }
    DBCFM& modAmt(float v){ mModAmt=v; return *this; }

    DBCFM& carMul2(float v){ mCarMul2=v; return *this; }
    DBCFM& modMul2(float v){ mModMul2=v; return *this; }
    DBCFM& modAmt2(float v){ mModAmt2=v; return *this; }

    DBCFM& idx1(float v){ mModEnv.levels()[0]=v; return *this; }
    DBCFM& idx2(float v){ mModEnv.levels()[1]=v; return *this; }
    DBCFM& idx3(float v){ mModEnv.levels()[2]=v; return *this; }

    DBCFM& set(float a, float b, float c, float d, float e, float f,
            float g, float h, float i, float j, float k, float l, float m, float n, float o, float p, float q, float r){
        return
            dur(a).freq(b).amp(c).amp2(d).attack(e).decay(f).attack2(g).decay2(h).sus(i).sus2(j)
        .idx1(k).idx2(l).idx3(m).carMul(n).modMul(o).carMul2(p).modMul2(q).pan(r);
    }

    virtual void onProcess(AudioIOData& io) override {

        float modFreq = mCarFrq * mModMul;
        float mod2Freq = mCarFrq * mModMul2;

        while(io()){
            car.freq(mCarFrq*mCarMul + mod()*mModEnv()*modFreq);
            car2.freq(mCarFrq*mCarMul2 + mod2()*mModEnv()*mod2Freq);
            float s1 = ((car()* mAmpEnv()* mAmp)   + (car2()* mAmpEnv2()* mAmp2));
            float s2;
            mEnvFollow(s1);
            mEnvFollow2(s1);
            mPan(s1, s1,s2);
            io.out(0) += s1;
            io.out(1) += s2;
        }

        if(mAmpEnv.done() && (mEnvFollow.value() < 0.001)) free();
        if(mAmpEnv2.done() && (mEnvFollow2.value() < 0.001)) free();
    }

    virtual void onTriggerOn() override {

        float modFreq = mCarFrq * mModMul;
        float mod2Freq = mCarFrq * mModMul2;
        mod.freq(modFreq);
        mod2.freq(mod2Freq);

        mAmpEnv.totalLength(mDur, 1);
        mAmpEnv2.totalLength(mDur, 1);
        //mModEnv.lengths()[1] = mAmpEnv.lengths()[1];
        //mModEnv.lengths()[1] = mAmpEnv2.lengths()[1];
        mModEnv.lengths()[1] = mAmpEnv.lengths()[3];
        mModEnv.lengths()[1] = mAmpEnv2.lengths()[3];

        mAmpEnv.reset();
        mAmpEnv2.reset();
        mModEnv.reset();
    }

protected:
    // general synth parameters
    //float mPitch; // implicit
    float mAmp;
    float mAmp2;
    float mDur;
    Pan<> mPan;

    // specific parameters
    float mCarFrq;        // carrier frequency
    float mCarMul;        // carrier frequency multiplier
    float mModMul;        // modulator frequency multiplier
    float mModAmt;        // frequency modulation amount
    float mCarMul2;        // carrier frequency multiplier
    float mModMul2;        // modulator frequency multiplier
    float mModAmt2;        // frequency modulation amount

    Sine<> car, car2, mod, mod2;    // carrier, modulator sine oscillators
    Env<3> mAmpEnv;
    Env<3> mAmpEnv2;
    Env<3> mModEnv;
    EnvFollow<> mEnvFollow;
    EnvFollow<> mEnvFollow2;
};


int main(){

    SynthSequencer s;
    s.add<DBCFM>( 0).set( 10, 262, 0.5, 0.5, 0.1, 5.1, 5.1, 0.1, 0.75, 0.75, 0.01, 7.0, 5.0,1.0, 1.0007,1,1.414, 1);
    s.add<DBCFM>(10).set( 10, 262, 0.5, 0.5, 5.1, 0.1, 0.1, 5.1, 0.81, 0.81, 1.7, 1.4, 1.1, 1.0, 1.998, 1.001, 1.414, 0);
    s.add<DBCFM>( 20).freq(220);
    s.add<DBCFM>( 30).set( 10, 262, 0.5, 0.5, 5.1, 0.1, 0.1, 5.1, 0.81, 0.81, 0.01,7,5, 1,1.0007, 2, 1.007, 0);
    s.add<DBCFM>( 40).set( 10, 262, 0.5, 0.5, 0.1, 3.1, 0.1, 3.1, 0.81, 0.81, 0.01,7,5, 1,1.0007, 1.0007, 5.008, -1);


    AudioIO io;
    io.initWithDefaults(s.audioCB, &s, true, false);
    Domain::master().spu(io.framesPerSecond());
    io.start();
    printf("\nPress 'enter' to quit...\n"); getchar();
}
