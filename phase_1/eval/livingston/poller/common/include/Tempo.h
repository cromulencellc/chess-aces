#ifndef TEMPO_H
#define TEMPO_H

#include "Utils.h"
#include "Serializable.h"

#include <iostream>

#define TEMPO_MAX    (1000)
#define TEMPO_MIN    (10)
#define BEAT_MAX     (Tempo::DOTTED_THIRTYSECOND)
#define BEAT_DEFAULT (Tempo::QUARTER)

/**
 * This class defines a tempo.
 */
class Tempo: public Serializable
{
public:

    /**
     * Denotes a beat quality.
     */ 
    enum Beat
    {
        WHOLE,
        HALF,
        QUARTER,
        EIGHTH,
        SIXTEENTH,
        THIRTYSECOND,
        DOTTED_WHOLE,
        DOTTED_HALF,
        DOTTED_QUARTER,
        DOTTED_EIGHTH,
        DOTTED_SIXTEENTH,
        DOTTED_THIRTYSECOND        
    };

    /**
     * Constructor
     */
    Tempo();

    /**
     * Constructor.
     * 
     * @param sampleRate The rate that samples occour.
     * @param bpm The beats per minutes, quarter gets the beat.
     */
    Tempo(uint32_t sampleRate, uint32_t bpm);

    /**
     * Returns the number of samples required to equal
     * the desired beat in the set tempo.
     * 
     * @param beat The beat to determine in samples.
     * @return The number of samples in the beat.
     */
    uint32_t getNumberOfSamples(Beat beat);

    /**
     * Returns the number of samples required to equal
     * the desired number of milliseconds.
     * 
     * @param msec The number of milliseconds to determine in samples.
     * @return The number of samples in the time intervals.
     */
    uint32_t getNumberOfSamples(uint32_t msec);

    /**
     * Returns the sample rate the tempo uses.
     * 
     * @return the sample rate.
     */
    uint32_t getSampleRate() { return this->sampleRate; }

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
    uint32_t bpm; 
    uint32_t sampleRate;
};


#endif