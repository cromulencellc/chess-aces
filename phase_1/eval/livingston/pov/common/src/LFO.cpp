#include "LFO.h"
#include "IDs.h"

#include <math.h>

LFO::LFO():
    detune(0), 
    modDepth(0), 
    rate(0), 
    carrier() 
{

}

LFO::LFO(double detune, double modDepth, double rate, uint32_t sampleRate): 
    detune(detune), 
    modDepth(modDepth), 
    rate(rate), 
    carrier(sampleRate) 
{

}

LFO::~LFO()
{

}

double LFO::modulateFrequency(double frequencyIn)
{
    double carrierSignal = this->carrier.getCarrierSignal(this->rate);
    double retVal = frequencyIn * ::pow(2, ((this->detune / 1200.0) + (carrierSignal * this->modDepth)));
    return retVal;
}

std::ostream &LFO::serialize(std::ostream &out) const
{
    out << this->detune;
    out << this->modDepth;
    out << this->rate;
    out << this->carrier;
    return out;
}

std::istream &LFO::dserialize(std::istream &in)
{
    std::string errorMessage = "Detected EOF early in chorus effect\n";
    ISTREAM_READ(in, this->detune,   errorMessage);
    ISTREAM_READ(in, this->modDepth, errorMessage);
    ISTREAM_READ(in, this->rate,     errorMessage);
        
    waveID w;
    ISTREAM_READ(in, w, errorMessage);
    if (w != SIN_I)
    {
        SystemException e("Expected a SIN wave for the LFO\n");
        throw e;
    }
    
    ISTREAM_READ(in, this->carrier, errorMessage);
    
    return in;
}