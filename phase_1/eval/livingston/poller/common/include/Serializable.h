#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H

#include "SystemException.h"

#include <iostream>

#define ISTREAM_READ(stream, target, err)   { \
                                                if (!stream.eof()) \
                                                { \
                                                    stream >> target; \
                                                } \
                                                else \
                                                { \
                                                    SystemException e(err); \
                                                    throw e; \
                                                } \
                                            } 

/**
 * Defines the interface for serialization.
 */
class Serializable
{
public:
    /**
     * Puts the serializable item into the stream. 
     * 
     * @param out The output stream
     * @param obj The object
     * @return The output stream
     */
    friend std::ostream &operator<<(std::ostream &out, const Serializable &obj) { return obj.serialize(out); }

    /**
     * Pulls the serializable item from the stream.
     * 
     * @param in The input stream
     * @param obj The object
     * @return The inout stream
     */
    friend std::istream &operator>>(std::istream &in, Serializable &obj) { return obj.dserialize(in); }

protected:
    /**
     * Serializes the object to the out stream.
     * 
     * @param out The out stream.
     * @return The out stream.
     */
    virtual std::ostream &serialize(std::ostream &out) const = 0;

    /**
     * Deserializes the object from istream.
     *
     * @param in The input stream
     * @return The input stream
     */
    virtual std::istream &dserialize(std::istream &in) = 0;
};

#endif