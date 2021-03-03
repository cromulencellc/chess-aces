#ifndef COMPRESSOR_EFFECT_H
#define COMPRESSOR_EFFECT_H

#include "EnvelopeShaperEffect.h"

/**
 * This class defines a compressor. 
 */
class CompressorEffect: public EnvelopeShaperEffect
{
public:
    /**
     * Constructor
     * 
     * @param tempo The tempo
     */
    CompressorEffect(Tempo &tempo);

    /**
     * Constructs a compressor.
     * 
     * @param priority The priority of the effect.
     * @param tempo The tempo.
     * @param attack The attack in milliseconds
     * @param release The release in milliseconds.
     * @param ratio The amount of compression.
     * @param threshold The max amplitude of the sample.
     */
    CompressorEffect(int priority, Tempo& tempo, uint32_t attack, uint32_t release, uint32_t ratio, uint32_t threshold);

    /**
     * Destructor
     */
    ~CompressorEffect();

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

    /**
     * Compresses the provided sample.
     * 
     * @param sample The sample to compress
     * @return The compressed sample.
     */
    audio_sample compressSample(audio_sample sample);
    
    double ratio;
    double threshold;
};

#endif