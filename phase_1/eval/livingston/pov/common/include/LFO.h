#ifndef LFO_H
#define LFO_H

#include "Utils.h"
#include "SinWave.h"
#include "Serializable.h"

/**
 * This class defines a low frequency oscilator. 
 */
class LFO: public Serializable
{
public:
    /**
     * Constructor.
     */
    LFO();

    /**
     * Constructor
     *
     * @param detune The amount of detune the oscilator will make
     * @param modDepth The depth of the effect.
     * @param rate The rate of the oscilator in hz
     */
    LFO(double detune, double modDepth, double rate, uint32_t sampleRate);
    
    /**
     * Destructor.
     */
    virtual ~LFO();

    /**
     * Gets a frequency that is modulated given the carrier position and modulation depth.
     * 
     * @param frequencyIn The unmodulated frequency
     * @return The modulated frequency.
     */
    double modulateFrequency(double frequencyIn);

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
    double detune;
    double modDepth;
    double rate;
    SinWave carrier;
};

#endif