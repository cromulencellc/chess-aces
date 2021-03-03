#include "TremoloEffect.h"
#include "IDs.h"

TremoloEffect::TremoloEffect(Tempo &tempo):
    Effect(),
    tempo(tempo)
{

}

TremoloEffect::TremoloEffect(int priority, Tempo &tempo, uint32_t mix, uint32_t rate, uint32_t mode):
    Effect(priority),
    tempo(tempo),
    mix(mix),
    mode(mode),
    rate(rate)
{

}

TremoloEffect::~TremoloEffect() 
{

}

void TremoloEffect::applyEffect(std::deque<audio_sample> &dry)
{
    double dep = this->mix;
    double offset = 1 - dep;
    int16_t counter_limit = this->rate;
    int16_t control = 1;
    int16_t mod = 0;

    for (int x = 0; x < dry.size(); ++x)
    {
        double clean = (double) dry[x];
        double dirty, modulation;

        modulation = (double) mod * dep / counter_limit;
        dirty = (modulation + offset)*clean;
        dry[x] = (audio_sample) dirty;

        mod += control;

	    if (mod > counter_limit) 
        {
	        control = -1;
        } 
        else if(!mod) 
        {
	        control = 1;
        }        
    }
}

std::ostream &TremoloEffect::serialize(std::ostream &out) const
{
    out << EFFECT_C;
    out << TREMOLO_C;
    std::ostream &o = Effect::serialize(out);
    o << this->mix;
    o << this->rate;
    o << this->mode;
    return o;
}


std::istream &TremoloEffect::dserialize(std::istream &in)
{
    std::string errorMessage = "Detected EOF early in tremolo effect\n";
    std::istream &i = Effect::dserialize(in);
    ISTREAM_READ(i, this->mix,  errorMessage);
    ISTREAM_READ(i, this->rate, errorMessage);
    ISTREAM_READ(i, this->mode, errorMessage);
    return i;
}