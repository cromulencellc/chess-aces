#ifndef SINWAVE_H
#define SINWAVE_H

#include "Wave.h"

/**
 * Class to generate sine wave data
 */
class SinWave: public Wave
{
public:
    /**
     * Constructor
     */
    SinWave();

    /**
     * Constructor.
     * 
     * @param sampleRate The rate at which this wave will be sampled.
     */
    SinWave(uint32_t sampleRate);

    /**
     * Destructor
     */
    virtual ~SinWave();

    virtual double getCarrierSignal(double freq);

    /**
     * Generates and returns the next sample.
     * 
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
    double curvePos;
};



#endif