#ifndef ENVELOPE_SHAPER_EFFECT_H
#define ENVELOPE_SHAPER_EFFECT_H

#include "Effect.h"
#include "Tempo.h"

/**
 * This class defines a envelope shaper effect. 
 */
class EnvelopeShaperEffect: public Effect
{
public:
    /**
     * Constructor.
     * 
     * @param tempo The tempo.
     */
    EnvelopeShaperEffect(Tempo& tempo);

    /**
     * Constructs an envelope shaper.
     * 
     * @param priority The priority of the effect.
     * @param tempo The tempo.
     * @param attack The attack in milliseconds
     * @param release The release in milliseconds.
     */
    EnvelopeShaperEffect(int priority, Tempo& tempo, uint32_t attack, uint32_t release);
    
    /**
     * Destructor
     */
    ~EnvelopeShaperEffect();

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

    /**
     * Returns the effect attack in millis.
     * 
     * @return The attack in millis
     */
    double getAttack() { return this->attack; }

    /**
     * Returns the effect release in millis.
     * 
     * @return The release in millis
     */
    double getRelease() { return this->release; }

    /**
     * Processes an audio sample.
     * 
     * @param sample The sample to process.
     * @return The processed sample.
     */
    double processSample(double sample);

    /**
     * Sets the attack of the shaper.
     * 
     * @param attack The effect attack in millis
     */
    void setAttack(double attack);

    /**
     * Sets the effect release.
     * 
     * @param release The effect release in millis
     */
    void setRelease(double release);

private:

    /**
     * Converts a value.
     * 
     * @param value The value to convert
     * @return The coinverted value
     */
    double convert(double value);

    Tempo &tempo;
    double attack;
    double release;
    double envelope;
};

#endif