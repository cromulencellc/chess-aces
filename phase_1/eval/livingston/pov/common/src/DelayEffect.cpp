#include "DelayEffect.h"
#include "IDs.h"

DelayEffect::DelayEffect(Tempo &tempo):
    Effect(),
    tempo(tempo),
    mix(0),
    delay(0),
    beatDelay((Tempo::Beat) 0),
    feedback(0)
{

}

DelayEffect::DelayEffect(int priority, Tempo &tempo, uint32_t mix, uint32_t delay, Tempo::Beat beatDelay, uint32_t feedback):
    Effect(priority),
    tempo(tempo),
    mix(mix),
    delay(delay),
    beatDelay(beatDelay),
    feedback(feedback)
{

}

DelayEffect::~DelayEffect()
{

}

void DelayEffect::applyEffect(std::deque<audio_sample> &dry)
{
    std::deque<audio_sample> temp(dry);
    double attenInt = 1.0 / (this->feedback + 1);
    double currentAtten = attenInt;
    double delay_sample, totalAttn;
    uint32_t numSamplesForDelay = 0;

    if (this->delay != 0)
    {
        numSamplesForDelay = this->tempo.getNumberOfSamples(this->delay);
    }
    else
    {
        numSamplesForDelay = this->tempo.getNumberOfSamples(this->beatDelay);  
    }
    
    int newSize = dry.size() + (feedback * numSamplesForDelay);
    dry.resize(newSize);

    int tracker = numSamplesForDelay;
    for (int fb = 0; fb < this->feedback && tracker < dry.size(); ++fb)
    {
        totalAttn = (1 - (currentAtten)) * (((double)this->mix) / 100);
        for (int y = 0; y < temp.size() && tracker + y < dry.size(); ++y)
        {
            delay_sample = ((double) temp[y]) * totalAttn;
            if (delay_sample != 0)
            {
                audio_sample a = dry[tracker + y];
                audio_sample b = (audio_sample) delay_sample;
                dry[tracker + y] = addSamples(a, b);
            }
        }  
        tracker += numSamplesForDelay;
        currentAtten += attenInt;
    }
}

std::ostream &DelayEffect::serialize(std::ostream &out)
{
    uint32_t bd = this->beatDelay;

    out << EFFECT_C;
    out << DELAY_C;
    std::ostream &o = Effect::serialize(out);   
    o << this->mix;
    o << this->delay;
    o << bd;
    o << this->feedback;
    return o;
}

std::istream &DelayEffect::dserialize(std::istream &in)
{
    uint32_t bd;
    std::string errorMessage = "Detected EOF early in delay effect\n";
    std::istream &i = Effect::dserialize(in);
    ISTREAM_READ(i, this->mix,       errorMessage);
    ISTREAM_READ(i, this->delay,     errorMessage);
    ISTREAM_READ(i, bd,              errorMessage);
    ISTREAM_READ(i, this->feedback,  errorMessage);

    if (bd > Tempo::DOTTED_THIRTYSECOND)
    {
        SystemException e("Malformed delay configuration.");
        throw e;
    }
    else
    {
        this->beatDelay = (Tempo::Beat) bd;
    }
    
    return i;
}