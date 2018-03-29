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


class Gliss : public SynthVoice {
public:

    Gliss()
//    :   mAmp(1), mDur(2)
    {
        set( 5, 0.5, 0.1, 0.08, 260.0, 540.0, 0.5, mGliss, 0.0);
        mAmpEnv.curve(0); // linear segments
        mAmpEnv.levels(0,1,1,0);
        mGlissEnv.curve(0);
    }

    Gliss& amp(float v){ mAmp=v; return *this; }
    Gliss& attack(float v){ mAmpEnv.lengths()[0]=v; return *this; }
    Gliss& decay (float v){ mAmpEnv.lengths()[2]=v; return *this; }
    Gliss& dur(float v){ mDur=v; return *this; }
    Gliss& pan(float v){ mPan.pos(v); return *this; }
    Gliss& table(ArrayPow2<float>& v){ mGliss.source(v); return *this; }

    Gliss& Gliss1(float v){ mGlissEnv.levels(v,mGlissEnv.levels()[1],v); return *this; }
    Gliss& Gliss2(float v){ mGlissEnv.levels()[1]=v; return *this; }
    Gliss& GlissRise(float v){ mGlissRise=v; return *this; }

    Gliss& set(
        float a, float b, float c, float d, float e, float f, float g, ArrayPow2<float>& h, float i
    ){
        return dur(a).amp(b).attack(c).decay(d)
            .Gliss1(e).Gliss2(f).GlissRise(g).table(h).pan(i);
    }

    // instrument Diagram Beginning
    virtual void onProcess(AudioIOData& io) override {
        while(io()){
            mGliss.freq(mGlissEnv());
            float s1 = mGliss() * mAmpEnv() * mAmp;
            float s2;
            mEnvFollow(s1);
            mPan(s1, s1,s2); // (signal in, split signal in1, signal s2
            io.out(0) += s1;
            io.out(1) += s2;
        }

        if(mAmpEnv.done() && (mEnvFollow.value() < 0.001)) free();
    }

    virtual void onTriggerOn() override {
        mAmpEnv.totalLength(mDur, 1);
        mGlissEnv.lengths()[0] = mDur * (1-mGlissRise);
        mGlissEnv.lengths()[1] = mDur * mGlissRise;
        mGlissEnv.reset();
        mAmpEnv.reset();
    }

protected:
    float mAmp;
    float mDur;
    Pan<> mPan;

    float mOscFrq;
    float mGlissFrq;
    float mGlissDepth;
    float mGlissRise;

    Osc<> mGliss;
    Env<3> mAmpEnv;
    Env<2> mGlissEnv;
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

    s.add<Gliss>( 0).set( 10.0, 0.5, 0.001, 0.03, 100, 1000.0, 0.5, tbSin, -1.0);
    //s.add<Gliss>( 10).Gliss1(110).Gliss2(440).table(tbSqr);

    AudioIO io(256, 44100., s.audioCB, &s);
    Domain::master().spu(io.framesPerSecond());
    io.start();
    printf("\nPress 'enter' to quit...\n"); getchar();
}
