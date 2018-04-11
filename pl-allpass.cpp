/*    Gamma - Generic processing library
    See COPYRIGHT file for authors and license information

    Example:        Filter / Plucked String
    Description:    Simulation of a plucked string with noise and a feedback
                    delay-line.
*/

#include <cstdio>               // for printing to stdout
#define GAMMA_H_INC_ALL         // define this to include all header files
#define GAMMA_H_NO_IO           // define this to avoid bringing AudioIO from Gamma

#include "Gamma/Gamma.h"

#include "al/core/io/al_AudioIO.hpp"
#include "al/util/ui/al_SynthSequencer.hpp"

using namespace gam;
using namespace al;

class PluckedString : public SynthVoice {
public:

    PluckedString(float frq=440)
        : mAmp(1), mDur(2),
          //comb(1./100, 1,0),
          mallpass(500,1000),
          env(0.1), fil(2), mdelay(1./27.5, 1./frq){
        decay(1.0);
        mAmpEnv.curve(4); // make segments lines
        mAmpEnv.levels(1,1,0);
    }

    PluckedString& freq(float v){mdelay.freq(v); return *this; }
    PluckedString& amp(float v){ mAmp=v; return *this; }
    PluckedString& dur(float v){
        mAmpEnv.lengths()[0] = v;
        return *this; }
    PluckedString& decay(float v){
        mAmpEnv.lengths()[1] = v;
        return *this;
    }
    //PluckedString& freq(float v){mallpass.freq(v); return *this; }
    //PluckedString& width(float v){mallpass.width(v); return *this; }
    PluckedString& pan(float v){ mPan.pos(v); return *this; }
    void reset(){ env.reset(); }

    PluckedString& set(
        float a, float b, float c, float d, float e=0

    ){
        return dur(a).freq(b).amp(c).decay(d).pan(e);

    }

    float operator() (){
        return (*this)(noise()*env());
    }

    float operator() (float in){
        return mdelay(
                     fil( mdelay() + in )
                     );
    }

    virtual void onProcess(AudioIOData& io) override {
        while(io()){
            float s = (*this)() * mAmpEnv() * mAmp;
            float s1 = mallpass(s);
            float s2;
            mEnvFollow(s1);
            mPan(s1, s1,s2);
            io.out(0) += s1;
            io.out(1) += s2;
        }
        if(mAmpEnv.done() && (mEnvFollow.value() < 0.001)) free();
    }


    virtual void onTriggerOn() override {
        mAmpEnv.reset();
    }

protected:
    float mAmp;
    float mDur;
    Pan<> mPan;
    NoiseWhite<> noise;
    Decay<> env;
    MovingAvg<> fil;
    Delay<float, ipl::Trunc> mdelay;
    Env<2> mAmpEnv;
    AllPass2<> mallpass;
    EnvFollow<> mEnvFollow;
};

int main(){

    SynthSequencer s;
    s.add<PluckedString>( 0  ).set(6.5, 110,  0.3, .005, 0);
    s.add<PluckedString>( 3.5  ).set(6.5, 110,  0.3, .005, 0);
    s.add<PluckedString>( 6.5).set(6.5, 233,  0.3, .1, 0);
    s.add<PluckedString>( 9.5).set(6.5, 233,  0.3, .1, 0);
    PluckedString &thirdPluck = s.add<PluckedString>( 12.5).set(6.5, 329,  0.7, .0001, 1);
//    s.add(Func(thirdPluck, &PluckedString::freq, 440)).dt(14);

    AudioIO io;
    io.initWithDefaults(s.audioCB, &s, true, false);
    Domain::master().spu(io.framesPerSecond());
    io.start();
    printf("\nPress 'enter' to quit...\n"); getchar();
}