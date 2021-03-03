#ifndef CHORUS_EFFECT_H
#define CHORUS_EFFECT_H

#define MAX_CHORUS_DELAY (500)

#include "Effect.h"
#include "Tempo.h"
#include "LFO.h"

/**
 * This class defines a chorus effect. 
 */
class ChorusEffect: public Effect
{
public:
    /**
     * Constructor.
     * 
     * @param tempo The tempo of the song.
     */
    ChorusEffect(Tempo &tempo);

    /**
     * Constructor
     * 
     * @param priority The priority of this effect.
     * @param tempo The tempo of the song.
     * @param mix The mix of dry signal and wet signal.
     * @param delay The offset of the wet signal.
     */
    ChorusEffect(int priority, Tempo &tempo, uint32_t mix, uint32_t delay,  uint32_t depth, uint32_t rate);

    /**
     * Destructor
     */
    virtual ~ChorusEffect();

    /**
     * Creates a new buffer with the effect applied.
     * 
     * @param dry The dry signal
     */
    virtual void applyEffect(std::deque<audio_sample> &dry);

protected:
    /**
     * Serializes the object to the out stream.
     * 
     * @param out The out stream.
     * @return The out stream.
     */
    virtual std::ostream &serialize(std::ostream &out) const;

    /**
     * Deserializes the object from istream.
     *
     * @param in The input stream
     * @return The input stream
     */
    virtual std::istream &dserialize(std::istream &in);    

private:
    Tempo &tempo;
    uint32_t mix;
    uint32_t delay;
    uint64_t depth;
    LFO lfo;
};

#endif
