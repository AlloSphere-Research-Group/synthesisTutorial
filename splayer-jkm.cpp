/*    Gamma - Generic processing library
    See COPYRIGHT file for authors and license information

    Example:
    Description:
*/

#include <cstdio>               // for printing to stdout
#define GAMMA_H_INC_ALL         // define this to include all header files
#include "Gamma/Gamma.h"

using namespace gam;


class SplayerEnv : public Process<AudioIOData> {
public:

    SplayerEnv(double dt=0)
    :    Process(dt), player("/Users/create/allocore9-27-12/AlloSystem/examples/joann.kucheramorin/good-isntrs-copy/flst.aif")

    {
        set (6.5, 60, 0.3, 1, 2);
        mAmpEnv.curve(0); // make segments lines
        mAmpEnv.levels(0,1,1,0);
    }

    SplayerEnv& amp(float v){ mAmp=v; return *this; }
    SplayerEnv& attack(float v){
        mAmpEnv.lengths()[0] = v;
        return *this;
    }
    SplayerEnv& decay(float v){
        mAmpEnv.lengths()[2] = v;
        return *this;
    }

    SplayerEnv& rate(float v){ player.rate(v); return *this; }

    SplayerEnv& dur(float v){ mDur = v; return *this; }

    SplayerEnv& pan(float v){ mPan.pos(v); return *this; }

    SplayerEnv& set(
        float a, float b, float c, float d, float e, float f=0
    ){
        return dur(a).amp(b).attack(c).decay(d).rate(e).pan(f);
    }

    //
    void onProcess(AudioIOData& io){

        mAmpEnv.totalLength(mDur, 1);

        while(io()){
            float s1 = player() * mAmpEnv() * mAmp;
            float s2 = 0;
            mEnvFollow(s1);
            mPan(s1, s1,s2);
            io.out(0) += s1;
            io.out(1) += s2;
        }
        //if(mAmpEnv.done()) free();
        if(mAmpEnv.done() && (mEnvFollow.value() < 0.001)) free();
    }


protected:
    float mAmp;
    float mDur;
    Pan<> mPan;
    Env<3> mAmpEnv;
    EnvFollow<> mEnvFollow;
    SamplePlayer<float, gam::ipl::Cubic, gam::tap::Wrap> player;
};



int main(){

    Scheduler s;
    s.add<SplayerEnv>( 0  ).set(10.5, 0.3, 1.011, .2, 1.0, 0);
    s.add<SplayerEnv>( 5  ).set(10.5, 0.4, 1.011, .2, 0.5, 0);
    s.add<SplayerEnv>( 10  ).set(10.5, 0.4, 1.011, .2, 0.25, 0);

    AudioIO io(256, 44100., Scheduler::audioCB, &s);
    Domain::master().spu(io.framesPerSecond());
    io.start();
    printf("\nPress 'enter' to quit...\n"); getchar();
}
