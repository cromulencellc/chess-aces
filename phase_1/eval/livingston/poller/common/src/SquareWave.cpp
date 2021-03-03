#include "SquareWave.h"
#include "IDs.h"

#include <math.h> 
#include <iostream>

SquareWave::SquareWave():
    Wave(),
    curvePos(0)
{

}

SquareWave::SquareWave(uint32_t sampleRate): Wave(sampleRate), curvePos(0)
{

}

SquareWave::~SquareWave()
{

}

double SquareWave::getCarrierSignal(double freq)
{
    double adjustedFreq = this->getOscilatedFrequency(freq);
    this->curvePos += adjustedFreq / this->getSampleRate();
    double sinApprox = ::sin(2 * PI * this->curvePos);
    double raw = sinApprox > 0 ? 1 : -1; 
    return raw;
}

audio_sample SquareWave::getSample(double freq)
{
    double adjustedFreq = this->getOscilatedFrequency(freq);
    this->curvePos += adjustedFreq / this->getSampleRate();
    double sinApprox = ::sin(2 * PI * this->curvePos);
    double raw = sinApprox > 0 ? 1 : -1; 
    return (audio_sample) (raw);
}

std::ostream &SquareWave::serialize(std::ostream &out) const 
{
    out << SQU_C;
    std::ostream &o = Wave::serialize(out);
    o << this->curvePos;
    return o;
}

std::istream &SquareWave::dserialize(std::istream &in) 
{
    std::string errorMessage = "Detected EOF early in SquareWave\n";
    std::istream &i = Wave::dserialize(in);
    ISTREAM_READ(i, this->curvePos, errorMessage);
    return i;
}