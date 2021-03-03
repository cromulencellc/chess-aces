#ifndef EFFECT_H
#define EFFECT_H

#include "Serializable.h"
#include "Utils.h"

#include <deque>
#include <iostream>

/**
 * This class defines the interface for working with effects. Each
 * effect can be given a priority that describes its place in a signal
 * chain.
 */
class Effect: public Serializable 
{
public:

    /**
     * Constructor
     * 
     * @param priority The number in an effect chain.
     */
    Effect(int priority) { this->priority = priority; }

    /**
     * Constructor
     */
    Effect() {}

    /**
     * Destructor
     */
    virtual ~Effect() {};

    /**
     * Creates a new buffer with the effect applied.
     * 
     * @param dry The dry signal
     * @return The signal with effect.
     */
    virtual void applyEffect(std::deque<audio_sample> &dry) = 0;

    /**
     * Returns the priority number of the effect.
     * 
     * @return The priority number
     */
    int getPriority() { return this->priority; }

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

    int priority;
};

#endif