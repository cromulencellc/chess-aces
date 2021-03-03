#ifndef TREMOLO_EFFECT_H
#define TREMOLO_EFFECT_H

#define MAX_TREMOLO_RATE     (10000)
#define MAX_TREMOLO_MODE_MAX (3)

#include "Effect.h"
#include "Tempo.h"

/**
 * This class defines a tremolo effect. 
 */
class TremoloEffect: public Effect
{
public:
    /**
     * Constructor
     * 
     * @param tempo The tempo
     */
    TremoloEffect(Tempo &tempo);

    /**
     * Constructs a tremolo effect.
     * 
     * @param priority The priority of the effect.
     * @param tempo The tempo of the song.
     * @param mix The wet mix.
     * @param rate The rate of the tremolo.
     * @param mode The waveform of the tremolo effect.
     */
    TremoloEffect(int priority, Tempo &tempo, uint32_t mix, uint32_t rate,  uint32_t mode);

    /**
     * Destructor
     */
    virtual ~TremoloEffect();

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
    uint32_t rate;
    uint32_t mode;
};

#endif