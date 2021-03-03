#ifndef WHITENOISE_H
#define WHITENOISE_H

#include "Wave.h"
#include <random>

/**
 * Class to generate white noise
 */
class WhiteNoise: public Wave
{
public:
    /**
     * Constructor
     */
    WhiteNoise();

    /**
     * Constructor.
     * 
     * @param sampleRate The rate at which this wave will be sampled.
     */
    WhiteNoise(uint32_t sampleRate);

    /**
     * Destructor
     */
    virtual ~WhiteNoise();

    /**
     * This should never be used as a carrier signal.
     * 
     * @param The frequency of the carrier.
     */
    virtual double getCarrierSignal(double freq);

    /**
     * Generates and returns the next sample.
     * 
     * @param amplitude
     * @param freq
     */
    virtual audio_sample getSample(double freq);

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
    std::mt19937 gen;
    std::uniform_real_distribution<> dis;
};



#endif