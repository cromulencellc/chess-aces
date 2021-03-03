#ifndef WAVE_H
#define WAVE_H

#include "Utils.h"
#include "Serializable.h"

/*
 * LFOs use waves and waves can use LFOs
 */
class LFO;

/**
 * Defines the base class of a wave. 
 */
class Wave: public Serializable
{
public:
    /**
     * Constructor 
     */
    Wave();

    /**
     * Constructor.
     * 
     * @param sampleRate The rate at which this wave will be sampled.
     */
    Wave(uint32_t sampleRate);

    /**
     * Destructor
     */
    virtual ~Wave();

    /**
     * Adds a low frequency oscilator to the waveform.
     * 
     * @param detune The detune amount of the oscilator.
     * @param modDepth The depth of the modulation.
     * @param rate The rate of the oscilator
     */
    void addLFO(double detune, double modDepth, double rate);

    /**
     * This allows the wave to be used as a carrier 
     * signal for oscilators and effects. This will
     * return the position along the curve of the wave 
     * without turning it to a sample.
     * 
     * @param The frequency of the carrier.
     */
    virtual double getCarrierSignal(double freq) = 0;

    /**
     * Generates and returns the next sample.
     * 
     * @param amplitude The amplitude of the signal.
     * @param freq The frequency to add to the wave
     */
    virtual audio_sample getSample(double freq) = 0;

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

protected:

    /**
     * Applies an oscilator, if any.
     * 
     * @param frequencyIn The frequency being generated.
     * @return The frequency with oscilation applied.
     */
    double getOscilatedFrequency(double frequencyIn);

    /**
     * Returns the sample rate for the wave.
     *
     * @return The sample rate for the file. 
     */
    uint16_t getSampleRate();

private:
    uint32_t sampleRate;
    LFO *lfo;
};

#endif