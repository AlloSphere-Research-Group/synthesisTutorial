
#ifndef POLYSYNTH_HPP
#define POLYSYNTH_HPP

#include <map>
#include <vector>
#include <list>
#include <limits.h>
#include <cassert>
#include <iostream>

#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/io/al_AudioIOData.hpp"

//#include "Gamma/Domain.h"

namespace al
{

template<class TSynthVoice> class PolySynth;

template<class TSynthVoice>
class SynthSequencerEvent {
public:
    double startTime {0};
    double duration {-1};
    int offsetCounter {0}; // To offset event within audio buffer
    TSynthVoice voice;
};

template<class TSynthVoice>
class SynthSequencer {
public:
    typedef enum {
        TIME_MASTER_AUDIO,
        TIME_MASTER_GRAPHICS
    } TimeMasterMode;

    SynthSequencer(unsigned int numPolyphony=32) : mPolySynth(numPolyphony)
    {
    }

    void render(AudioIOData &io) {
        if (mMasterMode == TIME_MASTER_AUDIO) {
            double timeIncrement = io.framesPerBuffer()/(double) io.framesPerSecond();
            mMasterTime += timeIncrement;
            if (mNextEvent < mEvents.size()) {
                auto iter = mEvents.begin();
                std::advance(iter, mNextEvent);
                auto event = *iter;
                while (event.startTime <= mMasterTime) {
                    event.offsetCounter = 0.5f + (event.startTime - (mMasterTime - timeIncrement))*io.framesPerSecond();
                    int id = mPolySynth.triggerOn(event.voice);
                    std::cout << "Event " << event.startTime << "-" << id << std::endl;
                    mNextEvent++;
                    iter++;
                    if (iter == mEvents.end()) {
                        break;
                    }
                    event = *iter;
                }
            }
        }
        mPolySynth.render(io);
    }

    void render(Graphics &g) {
        if (mMasterMode == TIME_MASTER_GRAPHICS) {
            double timeIncrement = 1.0/mFps;
            mMasterTime += timeIncrement;
            if (mNextEvent < mEvents.size()) {
                auto iter = mEvents.begin();
                std::advance(iter, mNextEvent);
                auto event = *iter;
                while (event.startTime <= mMasterTime) {
                    event.offsetCounter = 0;
                    int id = mPolySynth.triggerOn(event.voice);
                    std::cout << "Event " << event.startTime << "-" << id << std::endl;
                    mNextEvent++;
                    iter++;
                    if (iter == mEvents.end()) {
                        break;
                    }
                    event = *iter;
                }
            }
        }
        mPolySynth.render(g);
    }

    void setGraphicsFrameRate(float fps) {mFps = fps;} // TODO this should be handled through Gamma Domains

    TSynthVoice &add(double startTime, double duration = -1) {
        // Insert into event list, sorted.
        auto position = mEvents.begin();
        while(position != mEvents.end() && position->startTime < startTime) {
            position++;
        }
        auto insertedEvent = mEvents.insert(position, SynthSequencerEvent<TSynthVoice>());
        insertedEvent->startTime = startTime;
        insertedEvent->duration = duration;
        return insertedEvent->voice;
    }

    /// Basic audio callback for quick prototyping. Pass this audio callback to
    /// an AudioIO object with a pointer to a SynthSequencer instance to hear
    /// the sequence.
    static void audioCB(AudioIOData& io) {
        io.user<SynthSequencer<TSynthVoice>>().render(io);
    }

private:
    PolySynth<TSynthVoice> mPolySynth;

    double mFps {30}; // graphics frames per second

    unsigned int mNextEvent {0};
    std::list<SynthSequencerEvent<TSynthVoice>> mEvents; // List of events sorted by start time.

    TimeMasterMode mMasterMode {TIME_MASTER_AUDIO};
    double mMasterTime {0};
};

template<class TSynthVoice>
class PolySynth {
public:
    PolySynth(unsigned int numPolyphony=32) : mVoices(numPolyphony)
    {
        assert(numPolyphony < INT_MAX); // Polyphony needs to be smaller than INT_MAX
    }

    /// Triggers the start of a note event if a free note is available.
    /// Currently no voice stealing is implemented. Note is only triggered
    /// @returns id of the note triggered. -1 if no free voice is available
    int triggerOn(const TSynthVoice &newVoice) {
        for (int i = 0; i < (int) mVoices.size(); i++) {
            TSynthVoice &voice = mVoices[i];
            if (!voice.active()) {
                voice = newVoice;
                voice.triggerOn();
                return i;
            }
        }
        return -1;
    }

    ///
    void triggerOff(int id) {
        mVoices[id].triggerOff();
    }

    bool render(AudioIOData &io) {
        for (TSynthVoice &voice: mVoices) {
            if (voice.active()) {
                io.frame(voice.getStartOffsetFrames());
                int endOffsetFrames = voice.getEndOffsetFrames();
                if (endOffsetFrames >= 0) {
                    if (endOffsetFrames < io.framesPerBuffer()) {
                        voice.triggerOff(endOffsetFrames);
                    }
                    endOffsetFrames -= io.framesPerBuffer();
                }

                voice.onProcess(io);
            }
        }
    }

    bool render(Graphics &g) {
        for (TSynthVoice voice: mVoices) {
            if (voice.active()) {
                voice.onProcess(g);
            }
        }
    }

private:
    std::vector<TSynthVoice> mVoices;
};

class SynthVoice {
public:

    bool active() { return mActive;}

    /// Override this function to define audio processing. You will need to
    /// mark this instance as done by calling the free() function when
    /// envelopes or processing is done. You should call free() from one
    /// of the render() functions. You can access the note parameters using the
    /// getInstanceParameter(), getParameters() and getOffParameters() functions.
    virtual void onProcess(AudioIOData& io) {}

    /// Override this function to define graphics for this synth
    virtual void onProcess(Graphics &g) {}


    /// Define this function to determine what needs to be done when note/event
    /// starts. e.g. reset envelopes, etc.
    /// You can read the parameters using getInstanceParameter() or directly
    /// through getParameters()
    virtual void onTriggerOn() {}

    /// Define this function to determine what needs to be done when note/event
    /// ends. e.g. trigger release in envelopes, etc.
    virtual void onTriggerOff() {}

    /// This function can be called to programatically trigger  a voice.
    /// It is used for example in PolySynth to trigger a voice.
    void triggerOn(int offsetFrames = 0) {
        mOnOffsetFrames = offsetFrames;
        onTriggerOn();
        mActive = true;
    }

    /// This function can be called to programatically trigger the release
    /// of a voice.
    void triggerOff(int offsetFrames = 0) {
        mOffOffsetFrames = offsetFrames; // TODO implement offset frames for trigger off. Currently ignoring and turning off at start of buffer
        onTriggerOff();
    }

    /// Get the number of frames by which this voice should be offset within a
    /// processing block. This value is set to 0 once read as it should only
    /// apply on the first rendering pass of a voice.
    int getStartOffsetFrames() {
        int frames = mOnOffsetFrames;
        mOnOffsetFrames = 0;
        return frames;
    }

    int &getEndOffsetFrames() {return mOffOffsetFrames;}

protected:

    /// Mark this voice as done. This should be set within one of the render()
    /// functions when envelope or time is done and no more processing for
    /// the note is needed. The voice will be considered ready for retriggering
    /// by PolySynth
    void free() {mActive = false; } // Mark this voice as done.

private:
    int mActive {false};
    int mOnOffsetFrames {0};
    int mOffOffsetFrames {0};
};

}


#endif  // POLYSYNTH_HPP
