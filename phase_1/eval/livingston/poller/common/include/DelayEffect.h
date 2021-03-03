#ifndef DELAY_EFFECT_H
#define DELAY_EFFECT_H

#include "Effect.h"
#include "Tempo.h"

#define MAX_DELAY_FEEDBACK (5)
#define MAX_DELAY (1000)

/**
 * This class defines a delay effect. 
 */
class DelayEffect: public Effect
{
public:
    /**
     * Constructor
     * 
     * @param tempo The tempo.
     */
    DelayEffect(Tempo &tempo);

    /**
     * Constructor
     * 
     * @param priority The priority of this effect.
     * @param tempo The tempo of the song.
     * @param mix The mix of dry signal and wet signal.
     * @param delay The offset of the wet signal.
     * @param beatDelay The offset in beats (delay must be 0)
     * @param feedback The number of times the delay feeds back
     */
    DelayEffect(int priority, Tempo &tempo, uint32_t mix, uint32_t delay, Tempo::Beat beatDelay, uint32_t feedback);

    /**
     * Destructor
     */
    virtual ~DelayEffect();

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
    virtual std::ostream &serialize(std::ostream &out);

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
    Tempo::Beat beatDelay;
    uint32_t feedback;
};

#endif