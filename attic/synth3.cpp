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


class Vib : public SynthVoice {
public:

    Vib()
        : mAmp(1), mDur(2)
    {
        set( 5, 220, 0.5, 0.1, 0.08, 3.5, 5.8, 0.5, 0.005, mOsc, 0.0);
        mAmpEnv.curve(0); // linear segments
        mAmpEnv.levels(0,1,1,0);
        mVibEnv.curve(0);
    }

    Vib& freq(float v){ mOscFrq=v; return *this; }
    Vib& amp(float v){ mAmp=v; return *this; }
    Vib& attack(float v){ mAmpEnv.lengths()[0]=v; return *this; }
    Vib& decay (float v){ mAmpEnv.lengths()[2]=v; return *this; }
    Vib& dur(float v){ mDur=v; return *this; }
    Vib& pan(float v){ mPan.pos(v); return *this; }
    Vib& table(ArrayPow2<float>& v){ mOsc.source(v); return *this; }

    Vib& vibRate1(float v){ mVibEnv.levels(v,mVibEnv.levels()[1],v); return *this; }
    Vib& vibRate2(float v){ mVibEnv.levels()[1]=v; return *this; }
    Vib& vibDepth(float v){ mVibDepth=v; return *this; }
    Vib& vibRise(float v){ mVibRise=v; return *this; }

    Vib& set(
        float a, float b, float c, float d, float e, float f, float g, float h, float i, ArrayPow2<float>& j, float k
    ){
        return dur(a).freq(b).amp(c).attack(d).decay(e)
            .vibRate1(f).vibRate2(g).vibRise(h).vibDepth(i).table(j).pan(k);
    }

    //
    virtual void onProcess(AudioIOData& io) override {

        while(io()){
            mVib.freq(mVibEnv());
            mOsc.freq(mOscFrq + mVib()*mVibDepth*mOscFrq);

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


    virtual void onTriggerOn() override {
        mAmpEnv.totalLength(mDur, 1);
        mVibEnv.lengths()[0] = mDur * (1-mVibRise);
        mVibEnv.lengths()[1] = mDur * mVibRise;

        mAmpEnv.reset();
        mVibEnv.reset();
    }

protected:
    float mAmp;
    float mDur;
    Pan<> mPan;

    float mOscFrq;
    float mVibFrq;
    float mVibDepth;
    float mVibRise;

    Osc<> mOsc;
    Sine<> mVib;
    Env<3> mAmpEnv;
    Env<2> mVibEnv;
    EnvFollow<> mEnvFollow;
};



int main(){

    ArrayPow2<float>
        tbSaw(2048), tbSqr(2048), tbImp(2048), tbSin(2048), tbPls(2048),
        tb__1(2048), tb__2(2048), tb__3(2048), tb__4(2048);

    addSinesPow<1>(tbSaw, 9,1);
    addSinesPow<1>(tbSqr, 9,2);
    addSinesPow<0>(tbImp, 9,1);
    addSine(tbSin);

    {    float A[] = {1,1,1,1,0.7,0.5,0.3,0.1};
        addSines(tbPls, A,8);
    }

    {    float A[] = {1, 0.4, 0.65, 0.3, 0.18, 0.08};
        float C[] = {1,4,7,11,15,18};
        addSines(tb__1, A,C,6);
    }

    // inharmonic partials
    {    float A[] = {0.5,0.8,0.7,1,0.3,0.4,0.2,0.12};
        float C[] = {3,4,7,8,11,12,15,16};
        addSines(tb__2, A,C,7);
    }

    // inharmonic partials
    {    float A[] = {1, 0.7, 0.45, 0.3, 0.15, 0.08};
        float C[] = {10, 27, 54, 81, 108, 135};
        addSines(tb__3, A,C,6);
    }

    // harmonics 20-27
    {    float A[] = {0.2, 0.4, 0.6, 1, 0.7, 0.5, 0.3, 0.1};
        addSines(tb__4, A,8, 20);
    }


    SynthSequencer s;

    s.add<Vib>( 0).set( 10, 220, 0.5, 0.1, 0.08, 7.5, 7.5, 0.5, 0.05, tbSin, -1.0);
    //s.add<Vib>( 10).freq(110).table(tbSqr);
    s.add<Vib>(10).set( 10, 220, 0.5, 0.1, 0.08, 3.5, 10.8, 0.5, 0.03, tbSaw, 1.0);
    s.add<Vib>(20).set( 10, 110, 0.5, 0.1, 0.08, 9.8, 3.5, 0.5, 3.00, tbSqr, 0.0);
    s.add<Vib>(30).set( 10, 220, 0.3, 2.0, 1.00, 3.5, 15.0, 1.0, 0.005,tbSaw, -0.8);
    s.add<Vib>(40).set(10, 220, 0.3, 0.1, 0.08, 1.0, 100.0, 0.0, 0.30, tbSin, 0.8);
    s.add<Vib>(50).set( 40, 220, 0.5, 0.1, 0.08, .05, .00005, 0.5, 3.03, tbSaw, 1.0);

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
