#include "SinWave.h"
#include "IDs.h"

#include <math.h> 

SinWave::SinWave(): Wave(), curvePos(0)
{

}

SinWave::SinWave(uint32_t sampleRate): Wave(sampleRate), curvePos(0)
{

}

SinWave::~SinWave()
{

}

double SinWave::getCarrierSignal(double freq)
{
    double adjustedFreq = this->getOscilatedFrequency(freq);   
    this->curvePos += adjustedFreq / this->getSampleRate(); 
    return ::sin(2 * PI * this->curvePos);
}

audio_sample SinWave::getSample(double freq)
{
    double adjustedFreq = this->getOscilatedFrequency(freq);
    this->curvePos += adjustedFreq / this->getSampleRate();
    return (audio_sample) (::sin(2 * PI * this->curvePos));
}

std::ostream &SinWave::serialize(std::ostream &out) const 
{
    out << SIN_C;
    std::ostream &o = Wave::serialize(out);
    o << this->curvePos;
    return o;
}

std::istream &SinWave::dserialize(std::istream &in) 
{
    std::string errorMessage = "Detected EOF early in SinWave\n";
    std::istream &i = Wave::dserialize(in);
    ISTREAM_READ(i, this->curvePos, errorMessage);
    return i;
}