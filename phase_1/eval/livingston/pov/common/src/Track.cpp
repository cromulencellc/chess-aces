#include "Track.h"
#include "WaveFactory.h"
#include "EffectFactory.h"
#include "IDs.h"

Track::Track(Tempo &tempo):
    wave(NULL),
    tempo(tempo),
    volume(0),
    eq(NULL)
{
    for (int index = 0; index < MAX_EFFECTS; ++index)
    {
        this->effects[index] = NULL;
    }
}

Track::Track(Wave *wave, Tempo &tempo, double volume): 
    wave(wave), 
    tempo(tempo), 
    volume(volume),
    eq(NULL) 
{
    for (int index = 0; index < MAX_EFFECTS; ++index)
    {
        this->effects[index] = NULL;
    }
}

Track::~Track()
{
    delete this->wave;
    for (int index = 0; index < MAX_EFFECTS; ++index)
    {
        if (this->effects[index])
        {
            delete this->effects[index];
            this->effects[index] = NULL;
        }
    }
    this->eq = NULL;
}

void Track::addLFO(double detune, double modDepth, double rate)
{
    this->wave->addLFO(detune, modDepth, rate);
}

void Track::addNote(Semitone tone, int octave, Tempo::Beat beat) 
{
    double vol = 0;
    double freq =  getFreq(tone, octave);
    uint32_t samples = this->tempo.getNumberOfSamples(beat);

    for (uint32_t i = 0; i < samples; ++i)
    {
        // We want to attenuate a bit on the first and last
        // few samples to make attack less harsh. This should eliminate
        // that annoying click when a note starts or stops. 
        if (i < 100)
        {
            vol += 1 / 100;
        }
        
        if (i + 100 >= samples)
        {
            vol -= 1 / 100; 
        }

        audio_sample val = this->wave->getSample(freq);
        this->buffer.push_back(val);
    }
}

void Track::addRest(Tempo::Beat beat) 
{
    uint32_t samples = this->tempo.getNumberOfSamples(beat);
    
    for (uint32_t i = 0; i < samples; ++i)
    {
        this->buffer.push_back(0);
    }
}

bool Track::addEffect(Effect *effect)
{
    bool retVal = false;
    if (effect && effect->getPriority() < MAX_EFFECTS)
    {
        if (this->effects[effect->getPriority()])
        {
            delete this->effects[effect->getPriority()];
            this->effects[effect->getPriority()] = NULL;
        }
        this->effects[effect->getPriority()] = effect;
        retVal = true;
    }
    else
    {
        if (effect)
        {
            free(effect);
            effect = NULL;
        }
    }
    
    return retVal;
}

void Track::applyEffects()
{
    this->dirtyBuffer.clear();

    double scaler = dBToA(this->getVolume());

    for (int index = 0; index < this->getBuffer().size(); ++index)
    {
        audio_sample sample = this->getBuffer()[index];

        sample *= scaler;
        
        this->dirtyBuffer.push_back(sample);
    }

    for (int index = 0; index < MAX_EFFECTS; ++index)
    {
        if (this->effects[index])
        {
            this->effects[index]->applyEffect(this->dirtyBuffer);
        }
    }
}

std::deque<audio_sample>& Track::getEffectedBuffer()
{
    if (this->dirtyBuffer.size() > 0)
    {
        return this->dirtyBuffer;
    }
    return this->buffer;
}

Effect *Track::removeEffect(int priority)
{
    Effect *retVal = NULL;
    if (priority < MAX_EFFECTS)
    {
        retVal = this->effects[priority];
        this->effects[priority] = NULL;
    }

    if (retVal && this->eq && (retVal->getPriority() == this->eq->getPriority()))
    {
        this->eq = NULL;
    }

    return retVal;
}

bool Track::setEqualizer(EqualizerEffect *equalizer) 
{
    bool retVal = false;
    if (equalizer && equalizer->getPriority() < MAX_EFFECTS)
    {
        if (this->effects[equalizer->getPriority()])
        {
            delete this->effects[equalizer->getPriority()];
        }
        this->effects[equalizer->getPriority()] = equalizer;
        retVal = true;
    }
    else
    {
        if (equalizer)
        {
            free(equalizer);
            equalizer = NULL;
        }
    }
    
    if (retVal)
    {
        this->eq = equalizer;
    }
    
    return retVal;
}

uint64_t Track::getLengthInSamples()
{
    return this->buffer.size();
}

uint64_t Track::getLengthInMillis()
{
    uint64_t numSamplesPerMili = this->tempo.getNumberOfSamples(1);
    uint64_t numSamples = this->getEffectedBuffer().size();
    return numSamples / numSamplesPerMili;
}

std::ostream &Track::serialize(std::ostream &out) const 
{
    out << TRACK_C;
    out << this->wave;
    out << volume;

    uint32_t numEffects = 0;
    for (uint32_t x = 0; x < MAX_EFFECTS; ++x)
    {
        if (this->effects[x])
        {
            ++numEffects;
        }
    }
    out << numEffects;

    for (uint32_t x = 0; x < MAX_EFFECTS; ++x)
    {
        if (this->effects[x])
        {
            out << this->effects[x];
        }
    }

    uint32_t bufferSize = this->buffer.size();
    out << bufferSize;

    for (auto it = this->buffer.begin(); it != this->buffer.end(); ++it)
    {
        out << *it;
    }

    return out;
}

std::istream &Track::dserialize(std::istream &in) 
{
    uint32_t numEffects, bufferSize;
    WaveFactory wf(this->tempo.getSampleRate());
    std::istream &i = wf.deserializeWave(in, &(this->wave));
    std::string errorMessage = "Detected EOF early in Track\n";

    ISTREAM_READ(i, this->volume, errorMessage);
    ISTREAM_READ(i, numEffects, errorMessage);
    std::istream &i2 = deserializeEffectsForTrack(i, *this, this->tempo, numEffects);
    
    ISTREAM_READ(i2, bufferSize, errorMessage);

    for (uint32_t x = 0; x < bufferSize; ++x)
    {
        audio_sample s;
        ISTREAM_READ(i2, s, errorMessage);  
        this->buffer.push_back(s);   
    }

    return i2;
}
