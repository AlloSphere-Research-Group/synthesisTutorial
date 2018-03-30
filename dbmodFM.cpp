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


class DBMFM : public SynthVoice {
public:

    DBMFM()
        :   mAmp(1), mDur(2),
          mCarFrq(440), mCarMul(1), mModMul(1), mModMul2(1), mModAmt(50), mModAmt2(50)
    {
        set(4, 262, 0.5, 0.1, 0.3, 0.8,  1.2, 1.1, 0.9, 1.1, 0.9, 0.7, 1.0, 1.001, 3.001, 0);
    }

    DBMFM& freq(float v){ mCarFrq=v; return *this; }
    DBMFM& amp(float v){ mAmp=v; return *this; }
    DBMFM& attack(float v){
        mAmpEnv.lengths()[0] = v;
        mModEnv.lengths()[0] = v;
        mModEnv2.lengths()[0] = v;
        return *this;
    }
    DBMFM& decay(float v){
        mAmpEnv.lengths()[2] = v;
        mModEnv.lengths()[2] = v;
        mModEnv2.lengths()[2] = v;
        return *this;
    }

    DBMFM& sus(float v){
        mAmpEnv.levels()[2] = 1;
        return *this;
    }

    DBMFM& dur(float v){ mDur=v; return *this; }

    DBMFM& pan(float v){ mPan.pos(v); return *this; }

    DBMFM& carMul(float v){ mCarMul=v; return *this; }
    DBMFM& modMul(float v){ mModMul=v; return *this; }
    DBMFM& modAmt(float v){ mModAmt=v; return *this; }

    DBMFM& idx1(float v){ mModEnv.levels()[0]=v; return *this; }
    DBMFM& idx2(float v){ mModEnv.levels()[1]=v; return *this; }
    DBMFM& idx3(float v){ mModEnv.levels()[2]=v; return *this; }

    DBMFM& modMul2(float v){ mModMul2=v; return *this; }
    DBMFM& modAmt2(float v){ mModAmt2=v; return *this; }

    DBMFM& idx4(float v){ mModEnv2.levels()[0]=v; return *this; }
    DBMFM& idx5(float v){ mModEnv2.levels()[1]=v; return *this; }
    DBMFM& idx6(float v){ mModEnv2.levels()[2]=v; return *this; }


    DBMFM& set(float a, float b, float c, float d, float e, float f,
            float g, float h, float i, float j, float k, float l, float m, float n, float o, float p){
        return
            dur(a).freq(b).amp(c).attack(d).decay(e).sus(f).idx1(g).idx2(h).idx3(i).idx4(j).idx5(k).idx6(l)
        .carMul(m).modMul(n).modMul2(o).pan(p);
    }

    //
    virtual void onProcess(AudioIOData& io) override {

        float modFreq = mCarFrq * mModMul;
        float mod2Freq = mCarFrq * mModMul2;

        while(io()){
            //car.freq(mCarFrq*mCarMul + mod()*mModEnv()*modFreq) + (mCarFrq*mCarMul + mod2()*mModEnv2()*mod2Freq);
            car.freq(mCarFrq*mCarMul + mod()*mModEnv()*modFreq + mod2()*mModEnv2()*mod2Freq);
            float s1 = car() * mAmpEnv() * mAmp;
            float s2;
            mEnvFollow(s1);
            mPan(s1, s1,s2);
            io.out(0) += s1;
            io.out(1) += s2;
        }
        //if(mAmpEnv.done()) free();
        if(mAmpEnv.done() && (mEnvFollow.value() < 0.001)) free();
    }

    virtual void onTriggerOn() override {

        float modFreq = mCarFrq * mModMul;
        float mod2Freq = mCarFrq * mModMul2;
        mod.freq(modFreq);
        mod2.freq(mod2Freq);

        mAmpEnv.totalLength(mDur, 1);
        mModEnv.lengths()[1] = mAmpEnv.lengths()[3];
        mModEnv2.lengths()[1] = mAmpEnv.lengths()[3];

        mAmpEnv.reset();
        mModEnv.reset();
        mModEnv2.reset();
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
    float mModMul2;        // modulator frequency multiplier
    float mModAmt2;        // frequency modulation amount


    Sine<> car, mod, mod2;    // carrier, modulator sine oscillators
    Env<3> mAmpEnv;
    Env<3> mModEnv;
    Env<3> mModEnv2;
    EnvFollow<> mEnvFollow;
};

int main(){

    SynthSequencer s;
    s.add<DBMFM>( 0).set( 4, 262, 0.5, 0.1, 2.3, 0.8,  1.2, 1.1, 0.9, 1.1, 0.9, 0.7, 1.0, 1.001, 3.001, 0);
    s.add<DBMFM>( 4.1).freq(220);
    s.add<DBMFM>(8.2).set( 4, 220, 0.5, 0.005, 3.0, 0.8, 1.1, 0.9, 0.7, 1.2, 1.1, 0.9, 1.0, 2.7, 8.1, 0);
    s.add<DBMFM>(12.3).set(4, 262, 0.5, 0.1, 3.3, 0.8, 0.2, 1.1, 0.9, 0.1, 0.9, 0.7, 5.0, 1.001, 3.001, 0);
    s.add<DBMFM>(16.4).set( 4, 220, 0.5, 0.017, 3.0, 0.8, 1.2, 1.1, 0.9, 1.1, 0.9, 0.7, 1.0, 3.05, 4.08, 0);
    s.add<DBMFM>(20.5).set(4, 2322, 0.5, 0.0001, 3.0, 0.01, 1.1, 0.9, 1.1, 0.9, 0.7, 1.0, 1.05, 1.4, 1.2, 0);
    s.add<DBMFM>(24.6).set(4, 262, 0.5, 0.1, 0.3, 0.8, 1.2, 1.1, 0.9, 1.1, 0.9, 0.7, 3.0, 2.001, 1.001, 0);
    s.add<DBMFM>(28.8).set(4, 262, 0.5, 0.1, 0.3, 0.8, 1.2, 1.1, 0.9, 1.1, 0.9, 0.7, 3.0, 2.001, 0.301, 0);
    s.add<DBMFM>(32.1).set( 4, 262, 0.5, 0.1, 0.3, 0.8,  0.01, 7.0, 5.0, 1.1, 4.0, 4.0, 1.0, 1.007, 3.001, 0);
    s.add<DBMFM>(36.1).set( 4, 120, 0.5, 0.1, 0.3, 0.8,  0.01, 7.0, 5.0, 1.1, 4.0, 4.0, 1.0, 1.007, 3.001, 0);
    s.add<DBMFM>(40.1).set( 10, 60, 0.5, 0.1, 4.3, 0.8,  0.01, 7.0, 5.0, 1.1, 4.0, 4.0, 1.0, 1.007, 3.001, 0);
    s.add<DBMFM>(50.1).set( 20, 30, 0.5, 0.1, 10.3, 0.8,  0.01, 7.0, 5.0, 1.1, 4.0, 4.0, 1.0, 1.007, 3.001, 0);

    AudioIO io;
    io.init (s.audioCB, &s,256, 44100.);
    Domain::master().spu(io.framesPerSecond());
    io.start();
    printf("\nPress 'enter' to quit...\n"); getchar();
}
