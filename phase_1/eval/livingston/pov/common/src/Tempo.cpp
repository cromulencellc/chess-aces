#include "Tempo.h"

Tempo::Tempo():
    sampleRate(0),
    bpm(0)
{

}

Tempo::Tempo(uint32_t sampleRate, uint32_t bpm): sampleRate(sampleRate), bpm(bpm)
{

}
    
uint32_t Tempo::getNumberOfSamples(Beat beat)
{
    uint32_t retVal = 0;
    double scaler = 1;

    switch (beat)
    {
    case WHOLE:
        scaler = 240;
        break;
    case HALF:
        scaler = 120;    
        break;
    case QUARTER:
        scaler = 60;
        break;
    case EIGHTH:
        scaler = 30;       
        break;
    case SIXTEENTH:
        scaler = 15;    
        break;
    case THIRTYSECOND:
        scaler = 7.5;   
        break;
    case DOTTED_WHOLE:
        scaler = 360;
        break;
    case DOTTED_HALF:
        scaler = 180;
        break;
    case DOTTED_QUARTER:
        scaler = 90;
        break;
    case DOTTED_EIGHTH:
        scaler = 45;
        break;
    case DOTTED_SIXTEENTH:
        scaler = 22.5;
        break;
    case DOTTED_THIRTYSECOND:
        scaler = 11.25;
        break;
    }

    double bps = this->bpm / scaler;
    double spb = 1 / bps;
    double ret = spb * this->sampleRate;
    retVal = (uint32_t) ret;       
    return retVal;
}

uint32_t Tempo::getNumberOfSamples(uint32_t msec)
{
    double sps = (double) this->sampleRate;
    double spms = sps / 1000;
    return (uint32_t) spms * msec;
}

std::ostream &Tempo::serialize(std::ostream &out) const
{
    out << this->sampleRate;
    out << this->bpm;
    return out;
}

std::istream &Tempo::dserialize(std::istream &in)
{
    std::string errorMessage = "Detected EOF early in Tempo\n";
    ISTREAM_READ(in, this->sampleRate, errorMessage);
    ISTREAM_READ(in, this->bpm, errorMessage);
    return in;
}