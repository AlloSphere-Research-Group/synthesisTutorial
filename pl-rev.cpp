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

#include "allocore/io/al_AudioIO.hpp"
#include "allocore/ui/al_SynthSequencer.hpp"

using namespace gam;
using namespace al;

class PluckedString : public SynthVoice {
public:

    PluckedString(float frq=440)
    :   mAmp(1),
        comb(0.4, 1./100, 1,0),
        comb2(0.4, 1./200, 1,0),
        comb3(0.4, 1./300, 1,0),
        comb4(0.4, 1./400, 1,0),
        mallpass(1000,100),
        mallpass2(500,100),
        env(0.1), fil(2), delay(1./27.5, 1./frq){
        decay(1.0);
        mAmpEnv.curve(4); // make segments lines
        mAmpEnv.levels(1,1,0);
    }

    PluckedString& freq(float v){delay.freq(v); return *this; }
    PluckedString& amp(float v){ mAmp=v; return *this; }
    PluckedString& dur(float v){
        mAmpEnv.lengths()[0] = v;
        return *this; }
    PluckedString& decay(float v){
        mAmpEnv.lengths()[1] = v;
        return *this;
    }

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
        return delay(
                     fil( delay() + in )
                     );
    }

    void onProcess(AudioIOData& io){

        while(io()){
            //comb.feeds(0,-0.7);
            comb.feeds(0,-0.99);
            comb2.feeds(1,0);
            comb3.feeds(1,0);
            comb4.feeds(1,0);
            float s = (*this)() * mAmpEnv() * mAmp;
            comb.ipolType(ipl::CUBIC);
            comb2.ipolType(ipl::CUBIC);
            comb3.ipolType(ipl::CUBIC);
            comb4.ipolType(ipl::CUBIC);
            comb.maxDelay(0.4);
            comb2.maxDelay(0.4);
            comb3.maxDelay(0.4);
            comb4.maxDelay(0.4);
            comb.delay(1./100 + 1./10000);
            comb2.delay(1./200 + 1./10000);
            comb3.delay(1./300 + 1./10000);
            comb4.delay(1./400 + 1./10000);
            float s1 = (comb(s) + comb2(s) + comb3(s) + comb4(s));
            s1 += mallpass(s);
            s1 += mallpass2(s);
            //float s1 = comb(s);
            float s2;
            mPan(s1, s1,s2);
            mEnvFollow(s1);
            io.out(0) += s1;
            io.out(1) += s2;
        }
        if(mAmpEnv.done() && (mEnvFollow.value() < 0.00001f)) free();
    }

    virtual void onTriggerOn() override {
        mAmpEnv.reset();
    }

protected:
    float mAmp;
//    float mDur;
    Pan<> mPan;
    NoiseWhite<> noise;
    Decay<> env;
    MovingAvg<> fil;
    Delay<float, ipl::Trunc> delay;
    Env<2> mAmpEnv;
    //Comb<float, ipl::Any> comb;
    Comb<float, ipl::Linear> comb, comb2,comb3,comb4;
    AllPass2<> mallpass, mallpass2;
    EnvFollow<> mEnvFollow;
};

int main(){

    SynthSequencer s;
    s.add<PluckedString>( 0  ).set(6.5, 110,  0.3, .005, -1);
    s.add<PluckedString>( 3.5).set(6.5, 233,  0.3, .1, 0);
    PluckedString &thirdPluck = s.add<PluckedString>( 6.5).set(6.5, 329,  0.7, .0001, 1);
//    s.add(Func(thirdPluck, &PluckedString::freq, 440)).dt(8);

    AudioIO io(256, 44100., s.audioCB, &s);
    Domain::master().spu(io.framesPerSecond());
    io.start();
    printf("\nPress 'enter' to quit...\n"); getchar();
}
