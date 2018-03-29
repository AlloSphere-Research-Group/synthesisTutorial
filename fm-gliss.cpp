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
        mCarMul(1), mModMul(1), mModAmt(50)
    {
        //set( 5, 262, 0.5, 0.1,0.1, 0.75, 0.01,7,5, 1,1.0007, 0);
        mGlissEnv.curve(-4);
    }

    //FM& freq(float v){ mCarFrq=v; return *this; }
    FM& Gliss1(float v){ mGlissEnv.levels(v,mGlissEnv.levels()[1],v); return *this; }
    FM& Gliss2(float v){ mGlissEnv.levels()[1]=v; return *this; }
    FM& GlissRise(float v){ mGlissRise=v; return *this; }
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

    FM& set(float a, float b, float c, float d, float e, float f,
            float g, float h, float i, float j, float k, float l, float m, float n){
        return
            dur(a).Gliss1(b).Gliss2(c).GlissRise(d).amp(e).attack(f).decay(g).sus(h)
        .idx1(i).idx2(j).idx3(k)
        .carMul(l).modMul(m)
        .pan(n);
    }

    //
    virtual void onProcess(AudioIOData& io) override {

        mAmpEnv.totalLength(mDur, 1);
        mModEnv.lengths()[1] = mAmpEnv.lengths()[1];
        mGlissEnv.lengths()[0] = mDur * (1-mGlissRise);
        mGlissEnv.lengths()[1] = mDur * mGlissRise;

        //float modFreq = mGlissEnv() * mModMul;
        //mod.freq(modFreq);

        while(io()){
            //mGliss.freq(mGlissEnv()*mCarMul + mod()*mModEnv()*modFreq);

            float glissFreq = mGlissEnv();
            float carrierFreq = glissFreq * mCarMul;
            float modulatorFreq = glissFreq * mModMul;

            mod.freq( modulatorFreq );

            mGliss.freq(carrierFreq + (mod()*mModEnv()*modulatorFreq));

            float s1 = mGliss() * mAmpEnv() * mAmp;
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

        mAmpEnv.totalLength(mDur, 1);
        mModEnv.lengths()[1] = mAmpEnv.lengths()[1];
        mGlissEnv.lengths()[0] = mDur * (1-mGlissRise);
        mGlissEnv.lengths()[1] = mDur * mGlissRise;

        mAmpEnv.reset();
        mModEnv.reset();
        mGlissEnv.reset();
    }

protected:
    // general synth parameters
    //float mPitch; // implicit
    float mAmp;
    float mDur;
    Pan<> mPan;

    // specific parameters
    float mCarMul;        // carrier frequency multiplier
    float mModMul;        // modulator frequency multiplier
    float mModAmt;        // frequency modulation amount
    float mGlissFrq;
    float mGlissDepth;
    float mGlissRise;

    Sine<> mGliss, mod;    // carrier, modulator sine oscillators
    Env<3> mAmpEnv;
    Env<3> mModEnv;
    Env<2> mGlissEnv;
    EnvFollow<> mEnvFollow;
};


int main(){

    SynthSequencer s;
    s.add<FM>( 0).set( 5, 262, 440, 1.0, 0.5, 0.1,0.1, 0.75, 0.01,7,5, 1,1.0007, 1);//brass
    //s.add<FM>( 5).freq(220);//brass
    s.add<FM>(5).set( 5, 262, 440, 0.0, 0.5, 0.1,0.1, 0.75, 0.01,4,4, 3,2.0007,-1);//clarinet
    s.add<FM>(10).set( 5, 262, 440, 0.5, 0.5, 0.2,0.1, 0.75, 2.00,2,2, 3,1.0007, 0);//oboe
    s.add<FM>(15).set( 5, 139, 60, 0.0, 0.5, 0.2,0.1, 0.75, 0.01,1.5,1.5, 5,1.0007, 0);//bassoon
    s.add<FM>(20).set(10, 100, 300, 1.0, 0.5, 0.001,9.90, 0.8, 7.0, 7.0, 7.0, 1, 1.4, 0);//gong
    s.add<FM>(30).set(0.3,100, 500, 0.0, 0.5, 0.001,0.25, 0.8, 5,5,5, 1, 1.48, 0);//drum

    AudioIO io(256, 44100., s.audioCB, &s);
    Domain::master().spu(io.framesPerSecond());
    io.start();
    printf("\nPress 'enter' to quit...\n"); getchar();
}
