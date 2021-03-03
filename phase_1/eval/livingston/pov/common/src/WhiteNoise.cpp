#include "WhiteNoise.h"
#include "IDs.h"

#include <iostream>

WhiteNoise::WhiteNoise(): Wave()
{
    std::random_device rd;
    this->gen = std::mt19937(rd());
    this->dis = std::uniform_real_distribution<>(-1, 1);
}

WhiteNoise::WhiteNoise(uint32_t sampleRate): Wave(sampleRate)
{
    std::random_device rd;
    this->gen = std::mt19937(rd());
    this->dis = std::uniform_real_distribution<>(-1, 1);
}

WhiteNoise::~WhiteNoise()
{

}

double WhiteNoise::getCarrierSignal(double freq)
{
    return this->dis(this->gen);
}

audio_sample WhiteNoise::getSample(double freq)
{
    double raw = this->dis(this->gen);
    return (audio_sample) (raw);
}

std::ostream &WhiteNoise::serialize(std::ostream &out) const
{
    out << WHI_C;
    std::ostream &o = Wave::serialize(out);
    return o;
}

std::istream &WhiteNoise::dserialize(std::istream &in)
{
    std::string errorMessage = "Detected EOF early in White Noise\n";
    std::istream &i = Wave::dserialize(in);
    return i;
}