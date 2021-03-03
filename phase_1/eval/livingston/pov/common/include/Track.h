#ifndef TRACK_H
#define TRACK_H

#include "Note.h"
#include "Tempo.h"
#include "Wave.h"
#include "Effect.h"
#include "EqualizerEffect.h"
#include "Serializable.h"

#include <deque>

#define MAX_EFFECTS     (10)
#define MAX_TRACK_BOOST (100)
#define MAX_TRACK_CUT   (-100)

/**
 * This class defines a track. A track is a container for a wave generator
 * that combines the generator with a tempo. This makes for easy adding of 
 * notes and rests. Once the composition is finished, the buffer can be 
 * accessed for easy bouncing to file.
 */
class Track: public Serializable
{
public:
    /**
     * Constructor 
     * 
     * @param tempo The tempo
     */
    Track(Tempo &tempo);

    /**
     * Constructor. 
     *
     * @param wave The waveform of the track
     * @param tempo The tempo of the track
     * @param volume The volume of the track
     */
    Track(Wave *wave, Tempo &tempo, double volume);

    /**
     * Destructor
     */
    ~Track();

    /**
     * Adds an LFO to the track.
     * 
     * @param detune The detune amount in steps
     * @param modDepth The depth of the modulation in percent
     * @param rate The rate of the LFO
     */
    void addLFO(double detune, double modDepth, double rate);

    /**
     * Adds a note to the track.
     * 
     * @param tone The semitone to add
     * @param octave The octave to add
     * @param beat The duration of the note
     */
    void addNote(Semitone tone, int octave, Tempo::Beat beat);

    /**
     * Adds a rest to the track.
     * 
     * @param beat The duration of the rest
     */
    void addRest(Tempo::Beat beat);

    /**
     * Adds an effect to the effect chain.
     * 
     * @param effect The effect to add.
     * @return True on success
     */
    bool addEffect(Effect *effect);

    /**
     * Applies all effects to the track.
     */
    void applyEffects();

    /**
     * Returns a buffer that has had effects applied.
     * 
     * @return The effected data buffer.
     */
    std::deque<audio_sample>& getEffectedBuffer();

    /**
     * Gets the equalizer associated with the track, if any. If 
     * there is no equalizer, NULL is returned.
     * 
     * @return The equalizer.
     */
    EqualizerEffect *getEqualizer() { return this->eq; };

    /**
     * Gets the volume of the track.
     * 
     * @return The track volume
     */
    double getVolume() { return this->volume; }

    /**
     * Sets the equalizer associated with the track.
     * 
     * @param equalizer The equalizer.
     * @return True on success
     */
    bool setEqualizer(EqualizerEffect *equalizer);

    /**
     * Sets the volume. 
     * 
     * @param volume The volume in dB
     */
    void setVolume(double volume) { this->volume = volume; }

    /**
     * Removes the effect at the specified priority.
     * 
     * @param priority
     * @return The effect removed
     */
    Effect *removeEffect(int priority);

    /**
     * Returns the number of samples in the track.
     * 
     * @return The number of samples.
     */
    uint64_t getLengthInSamples();

    /**
     * Returns the length of the track in milliseconds. 
     * 
     * @return The track length in milliseconds
     */
    uint64_t getLengthInMillis();

protected:
    /**
     * Serializes the object to the out stream.
     * 
     * @param out The out stream.
     * @return The out stream.
     */
    virtual std::ostream &serialize(std::ostream &out) const;

    /**
     * Deserializes the object from istream.
     *
     * @param in The input stream
     * @return The input stream
     */
    virtual std::istream &dserialize(std::istream &in);

private:
    /**
     * Returns the buffer that represents the track.
     * 
     * @return The track buffer
     */
    std::deque<audio_sample>& getBuffer() { return this->buffer; }

    /**
     * Returns the effected buffer.
     * 
     * @return the effected buffer.
     */
    std::deque<audio_sample>& getDirtyBuffer() { return this->dirtyBuffer; }

    Wave *wave;
    Tempo &tempo;
    double volume;
    Effect *effects[MAX_EFFECTS];
    EqualizerEffect *eq;
    std::deque<audio_sample> buffer;
    std::deque<audio_sample> dirtyBuffer;
};


#endif