#ifndef SAWWAVE_H
#define SAWWAVE_H

#include "Wave.h"

/**
 * Class to generate saw wave data
 */
class SawWave: public Wave
{
public:
    /**
     * Constructor
     */
    SawWave();

    /**
     * Constructor.
     * 
     * @param sampleRate The rate at which this wave will be sampled.
     */
    SawWave(uint32_t sampleRate);

    /**
     * Destructor
     */
    virtual ~SawWave();

    /**
     * This allows the wave to be used as a carrier 
     * signal for oscilators and effects. This will
     * return the position along the curve of the wave 
     * without turning it to a sample.
     * 
     * @param The frequency of the carrier.
     */
    virtual double getCarrierSignal(double freq);

    /**
     * Generates and returns the next sample.
     * 
     * @param freq The frequency to produce
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