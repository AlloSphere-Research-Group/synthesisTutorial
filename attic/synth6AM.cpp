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
#include "al/util/scene/al_SynthSequencer.hpp"

using namespace gam;
using namespace al;

class OscAM : public SynthVoice {
public:

    OscAM()
        : mAmp(1), mDur(2)
    {
        mAmpEnv.levels(0,1,1,0);
        mAMEnv.curve(0);
    }

    OscAM& freq(float v){ mOsc.freq(v); return *this; }
    OscAM& amp(float v){ mAmp=v; return *this; }
    OscAM& dur(float v){ mDur=v; return *this; }
    OscAM& attack(float v){ mAmpEnv.lengths()[0]=v; return *this; }
    OscAM& decay(float v){  mAmpEnv.lengths()[2]=v; return *this; }
    OscAM& sus(float v){ mAmpEnv.levels()[2]=v; return *this; }

    OscAM& am1(float v){ mAMEnv.levels(v, mAMEnv.levels()[1], v); return *this; }
    OscAM& am2(float v){ mAMEnv.levels()[1]=v; return *this; }
    OscAM& amRatio(float v){ mAMRatio=v; return *this; }
    OscAM& amRise(float v){ mAMEnv.lengths(v,1-v); return *this; }
    OscAM& amFunc(ArrayPow2<float>& v){ mAM.source(v); return *this; }

    OscAM& pan(float v){ mPan.pos(v); return *this; }


    OscAM& set(
        float a, float b, float c, float d, float e, float f,
        float g, float h, float i, float j, ArrayPow2<float>& k,
        float l=0
    ){
        return dur(a).freq(b).amp(c).attack(d).decay(e).sus(f)
            .am1(g).am2(h).amRise(i).amRatio(j).amFunc(k)
            .pan(l);
    }

    virtual void onProcess(AudioIOData& io) override {

        while(io()){

            mAM.freq(mOsc.freq()*mAMRatio);            // set AM freq according to ratio
            float amAmt = mAMEnv();                    // AM amount envelope

            float s1 = mOsc();                        // non-modulated signal
            s1 = s1*(1-amAmt) + (s1*mAM())*amAmt;    // mix modulated and non-modulated

            s1 *= mAmpEnv() *mAmp;

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
        mAMEnv.totalLength(mDur);

        mAmpEnv.reset();
        mAMEnv.reset();
    }

protected:
    float mAmp;
    float mDur;
    Pan<> mPan;

    Osc<> mAM;
    float mAMRatio;
    Env<2> mAMEnv;
    Sine<> mOsc;
    Env<3> mAmpEnv;
    EnvFollow<> mEnvFollow;
};


int main(){

    ArrayPow2<float> tbSin(2048), tbSqr(2048), tbPls(2048), tbDin(2048);
    addWave(tbSin, SINE);
    addWave(tbSqr, SQUARE);
    addWave(tbPls, IMPULSE, 4);

    // inharmonic partials
    {    float A[] = {1, 0.7, 0.45, 0.3, 0.15, 0.08};
        float C[] = {10, 27, 54, 81, 108, 135};
        addSines(tbDin, A,C,6);
    }

    SynthSequencer s;
    s.add<OscAM>( 0).set(5, 262, 0.5, 0.1,0.08,0.8, 0.2,0.8,0.5, 1.4870, tbSin);
    s.add<OscAM>( 5).set(10, 262, 0.5, 0.1,0.08,0.8, 0.0,1.0,0.0, 2.0001, tbSin);
    s.add<OscAM>(15).set(5, 262, 0.5, 0.1,0.08,0.8, 0.2,0.8,1.0, 2.0001, tbPls);
    s.add<OscAM>(20).set(5, 262, 0.5, 0.1,0.08,0.8, 0.2,0.8,1.0, 2.0001/10., tbDin);

    AudioIO io;
    io.initWithDefaults(s.audioCB, &s, true, false);
    Domain::master().spu(io.framesPerSecond());
    io.start();
    printf("\nPress 'enter' or Ctrl-C to quit...\n");
    while (io.isRunning()) {
      auto c = getchar();
      if (c != -1) {
        std::cout << c <<std::endl;
        io.stop();
      }
    }
    return 0;
}
