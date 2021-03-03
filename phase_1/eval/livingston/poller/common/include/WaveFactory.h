#ifndef WAVEFACTORY_H
#define WAVEFACTORY_H

#include "Wave.h"

#define MAX_WAVE_TYPE (WaveFactory::WHITE)

/**
 * Factory for generation of waves.
 */
class WaveFactory
{
public:     

    /**
     * Identifies the type
     * of wave that will be
     * generated.
     */
    enum WaveType
    {
        SINE,
        SQUARE,
        SAW,
        TRIANGLE,
        WHITE
    };
   
    /**
     * Constructor
     */
    WaveFactory();

    /**
     * Constructor
     * 
     * @param sampleRate The sample rate for all generated waves
     */
    WaveFactory(uint32_t sampleRate);

    /**
     * Generates a wave.
     * 
     * @param waveType The type of wave to generate.
     * @return A wave.
     */
    Wave *getWave(WaveType waveType);

    /**
     * Deserializes a wave from the input stream.
     * 
     * @param in The input stream.
     * @param wave The out value of the wave.
     * @return the stream.
     */
    std::istream &deserializeWave(std::istream &in, Wave **wave);

    /**
     * Sets the sample rate.
     * 
     * @param sampleRate The new sample rate
     */
    void setSampleRate(uint32_t sampleRate) { this->sampleRate = sampleRate; }
    
private:
    uint32_t sampleRate;    
};


#endif