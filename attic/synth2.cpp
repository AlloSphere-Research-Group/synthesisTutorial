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


class OscEnv : public SynthVoice {
public:

    OscEnv()
    :   mAmp(1), mDur(2)

    {

        set( 4, 262, 0.3, 0.1 , 0.075, 0.7, 4, mOsc, 0.2);
        mAmpEnv.levels(0,1,1,0);
    }

    OscEnv& freq(float v){ mOsc.freq(v); return *this; }
    OscEnv& amp(float v){ mAmp=v; return *this; }
    OscEnv& dur(float v){ mDur=v; return *this; }
    OscEnv& attack(float v){ mAmpEnv.lengths()[0]=v; return *this; }
    OscEnv& decay(float v){  mAmpEnv.lengths()[2]=v; return *this; }
    OscEnv& sus(float v){ mAmpEnv.levels()[2]=v; return *this; }
    OscEnv& curve(float v){ mAmpEnv.curve(v); return *this; }
    OscEnv& pan(float v){ mPan.pos(v); return *this; }
    OscEnv& table(ArrayPow2<float>& v){ mOsc.source(v); return *this; }

    OscEnv& set(
        float a, float b, float c, float d, float e, float f, float g, ArrayPow2<float>& h, float i
    ){
        return dur(a).freq(b).amp(c).attack(d).decay(e).sus(f).curve(g).table(h).pan(i);
    }

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
        if(mAmpEnv.done() && (mEnvFollow.value() < 0.001)) free();
    }

    virtual void onTriggerOn() override {
        mAmpEnv.totalLength(mDur, 1);
        mAmpEnv.reset();
    }

protected:
    float mAmp;
    float mDur;
    Pan<> mPan;
    Osc<> mOsc;
    Env<3> mAmpEnv;
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
        addSines(tb__2, A,C,8);
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
    s.add<OscEnv>( 0).set( 4, 262, 0.3, 0.1 , 0.075, 0.7, 4, tbSin, 0.2);
    s.add<OscEnv>( 2).freq(220).table(tbSqr);
    s.add<OscEnv>( 4).set( 4, 262, 0.3, 2.0 , 0.3  , 0.7, 0, tbSaw,-0.2);
    s.add<OscEnv>( 8).set( 4, 262, 0.3, 0.1 , 0.075, 0.7, 4, tbSqr, 0.0);
    s.add<OscEnv>(12).set( 4, 262, 0.3, 3.0 , 0.3  , 0.9, 0, tbPls, 0.0);
    s.add<OscEnv>(16).set( 4, 262, 0.3, 0.1 , 2    , 0.7, 4, tb__1,-0.6);
    s.add<OscEnv>(20).set( 4, 262, 0.3, 0.15, 0.3  , 0.5, 0, tb__2, 0.0);
    s.add<OscEnv>(24).set( 4, 26.2, 0.3, 0.1 , 0.075, 0.7, 4, tb__3, 0.6);
    s.add<OscEnv>(28).set( 4, 262, 0.3, 0.15, 0.3  , 0.5, 0, tb__4, 0.0);
    s.add<OscEnv>(32).set(10, 262, 0.1, 0.1 , 0.075, 0.7, 4, tbSin, 0.2);
    s.add<OscEnv>(32).set(10, 262, 0.1, 2   , 0.3  , 0.7, 0, tbSaw,-0.2);
    s.add<OscEnv>(32).set(10, 263, 0.1, 0.1 , 0.075, 0.7, 4, tbSqr, 1.0);
    s.add<OscEnv>(32).set(10, 261, 0.1, 3   , 0.3  , 0.9, 0, tbPls,-1.0);
    s.add<OscEnv>(32).set(10, 262.5,0.1,0.1 , 2    , 0.7, 4, tb__1, 0.0);
    s.add<OscEnv>(32).set(10, 262, 0.1, 0.15, 0.3  , 0.5, 0, tb__2, 0.0);
    s.add<OscEnv>(32).set(10, 26.4, 0.1, 0.1 , 0.075, 0.7, 4, tb__3, 0.6);
    s.add<OscEnv>(32).set(10, 265, 0.1, 0.15, 0.3  , 0.5, 0, tb__4, 0.7);


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
