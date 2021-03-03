#include "CompressorEffect.h"
#include "IDs.h"

#include <cmath>

CompressorEffect::CompressorEffect(Tempo &tempo):
    EnvelopeShaperEffect(tempo),
    ratio(0),
    threshold(0)
{

}

CompressorEffect::CompressorEffect(int priority, Tempo& tempo, uint32_t attack, uint32_t release, uint32_t ratio, uint32_t threshold):
    EnvelopeShaperEffect(priority, tempo, attack, release),
    ratio(ratio),
    threshold(threshold)
{

}

CompressorEffect::~CompressorEffect() 
{

}

void CompressorEffect::applyEffect(std::deque<audio_sample> &dry) 
{
    for (int index = 0; index < dry.size(); ++index)
    {
        dry[index] = this->compressSample(dry[index]);
    }
}    

audio_sample CompressorEffect::compressSample(audio_sample sample)
{
    audio_sample retVal = sample;

    double proc = (double) sample;
    proc = ::fabs(proc);
    proc = this->processSample(proc);
    proc = aToDB(proc);
    
    if (proc > this->threshold)
    {
        double scalar = 1.00 - (1.00 / this->ratio);
        double gain = scalar * (this->threshold - proc);
        gain = dBToA(gain);
        retVal *= gain;
    }

    return retVal;
}

std::ostream &CompressorEffect::serialize(std::ostream &out) const
{
    out << EFFECT_C;
    out << COMPRESS_C;
    std::ostream &o = EnvelopeShaperEffect::serialize(out);
    o << this->ratio;
    o << this->threshold;
    return o;
}

std::istream &CompressorEffect::dserialize(std::istream &in)
{
    typeID t;
    std::string errorMessage = "Detected EOF early in effect\n";

    ISTREAM_READ(in, t, errorMessage);
    if (t == EFFECT_I)
    {
        effectID eid;
        ISTREAM_READ(in, eid, errorMessage);
        if (eid == ENVELOPE_I)
        {
            std::istream &i = EnvelopeShaperEffect::dserialize(in);
            ISTREAM_READ(i, this->ratio,     errorMessage);
            ISTREAM_READ(i, this->threshold, errorMessage);
            return i;
        }
        else
        {
            SystemException e("Expected a nested envelope shaper in compressor effect.\n");
            throw e;
        }
    }
    else
    {
        SystemException e("Expected nested effect in compressor effect.\n");
        throw e;
    }

    return in;
}