#include "ChorusEffect.h"
#include "IDs.h"

ChorusEffect::ChorusEffect(Tempo &tempo):
    Effect(),
    tempo(tempo),
    mix(0),
    delay(0),
    lfo()
{
    
}

ChorusEffect::ChorusEffect(int priority, Tempo &tempo, uint32_t mix, uint32_t delay, uint32_t depth, uint32_t rate): 
    Effect(priority),
    tempo(tempo),
    mix(mix), 
    delay(delay),
    lfo(0.0, depth, rate, tempo.getSampleRate())
{

}

ChorusEffect::~ChorusEffect() 
{

}

void ChorusEffect::applyEffect(std::deque<audio_sample> &dry)
{
    std::deque<audio_sample> temp;  
    uint32_t msecLeadin = this->tempo.getNumberOfSamples(this->delay);
  
    double wetMixScaler = (this->mix / 100.00);
    double dryMixScaler = 1 - wetMixScaler;

    for (int index = 0; index < dry.size(); ++index)
    {
        temp.push_back(dry[index] * wetMixScaler);
        dry[index] *= (dryMixScaler);
    }
    
    for (int index = 0; index < msecLeadin; ++index)
    {
        dry.push_back(0);
    }

    for (int index = 0; index < temp.size(); ++index)
    {
        if (temp[index] != 0)
        {
            audio_sample a, b;
            a = dry[index + msecLeadin];
            b = (audio_sample) this->lfo.modulateFrequency(temp[index]);
            dry[index + msecLeadin] = addSamples(a, b);
        }
    }
}

std::ostream &ChorusEffect::serialize(std::ostream &out) const
{
    out << EFFECT_C;
    out << CHORUS_C;
    std::ostream &o = Effect::serialize(out);
    o << this->mix;
    o << this->delay;
    o << this->depth;
    o << this->lfo;
    return o;
}

std::istream &ChorusEffect::dserialize(std::istream &in)
{
    std::string errorMessage = "Detected EOF early in chorus effect\n";
    std::istream &i = Effect::dserialize(in);
    ISTREAM_READ(i, this->mix,   errorMessage);
    ISTREAM_READ(i, this->delay, errorMessage);
    ISTREAM_READ(i, this->depth, errorMessage);
    ISTREAM_READ(i, this->lfo,   errorMessage);

    if (this->delay >MAX_CHORUS_DELAY)
    {
        SystemException e("Malformed chorus configuration\n");
        throw e;
    }

    return i;
}   