#ifndef SONG_H
#define SONG_H

#include "Track.h"
#include "WaveFactory.h"
#include "WaveFile.h"
#include "Serializable.h"

#include <deque>
#include <string>

/**
 * This class defines the elements of a song
 */
class Song: public Serializable
{
public:
    /**
     * Constructor
     */
    Song();

    /**
     * Constructor
     *
     * @param sampleRate The sample rate of the song.
     * @param bpm The tempo of the song, quarter gets the beat.
     * @param name The name of the song.
     */
    Song(uint32_t sampleRate, uint32_t bpm, std::string name);

    /**
     * Destructor
     */
    ~Song();

    /**
     * Adds a not of the given duration to a track.
     * 
     * @param trackNumber The track that will get the note.
     * @param note The semitone that will be added.
     * @param octave The octave of the note being added.
     * @param duration The duration of the note.
     */
    void addNote(uint32_t trackNumber, Semitone note, int octave, Tempo::Beat duration);

    /**
     * Adds a rest of the given duration to the specified track.
     * 
     * @param trackNumber The track that will get the rest.
     * @param duration The length of the rest.
     */
    void addRest(uint32_t trackNumber, Tempo::Beat duration);

    /**
     * Adds an effect to the given track.
     * 
     * @param trackNumber The track number to add the effect to.
     * @param effect The effect.
     */
    void addEffect(uint32_t trackNumber, Effect *effect);

    /**
     * Adds an equalizer to the specfied track.
     * 
     * @param trackNumber The track to add the equalizer to.
     * @param eq The equalizer to add.
     */
    void addEqualizer(uint32_t trackNumber, EqualizerEffect *eq);

    /**
     * Adds a track to the song. The track will have the 
     * waveform specified. The number of the track will be
     * returned for later referencing. The volume should be
     * provided as a value in [0, 1].
     * 
     * @param waveType The wave form for the new track.
     * @param volume the volume of the track.
     * @return The tack number for later referencing.
     */
    uint32_t addTrack(WaveFactory::WaveType waveType, double volume);

    /**
     * Adds LFO to a track.
     * 
     * @param trackNumber The track number
     * @param detune The detune amount
     * @param modDepth The depth
     * @param rate The rate of the oscilation
     */
    void addLFO(uint32_t trackNumber, double detune, double modDepth, double rate);

    /**
     * Writes the song contents to the buffer provided.
     * 
     * @param buffer the buffer to write.
     */
    void bounceToBuffer(std::ostream& buffer);

    /**
     * Writes the song contents to the buffer provided.
     *
     * @param buffer the buffer to write.
     * @return The size of the buffer.
     */
    size_t bounceToBuffer(uint8_t **buffer);

    /**
     * Bounces the song to a file. The file will be a 
     * wave fille named after the song.
     */
    void bounceToFile();

    /**
     * Gets the track from the specified track.
     * 
     * @param trackNumber The track to get the equalizer from.
     * @return The equalizer for the track.
     */
    EqualizerEffect *getEqualizer(uint32_t trackNumber);

    /**
     * Gets the length of the song in samples.
     * 
     * @return The length of the tra
     */
    uint32_t getLengthSamples();

    /**
     * Gets the length of the song in milliseconds.
     * 
     * @return The length of the song in millis.
     */
    uint32_t getLengthMillis();

    /**
     * Gets the number of tracks.
     * 
     * @return The number of tracks.
     */
    uint32_t getNumTracks() { return this->tracks.size(); }

    /**
     * Returns the song name.
     * 
     * @return The song name.
     */
    std::string &getSongName() { return this->name; }

    /**
     * Gets the tempo of the song.
     * 
     * @return The tempo of the song.
     */
    Tempo &getTempo() { return this->tempo; }

    /**
     * Removes the effect with the given priority.
     * 
     * @param trackNumber The track number to remove the effect from
     * @param priority The priority of the effect to remove.
     */
    void removeEffect(uint32_t trackNumber, int priority);

    /**
     * Sets the volume of the given track.
     * 
     * @param trackNumber The track to set.
     * @param gain The gain boost/cut
     */
    void setVolume(uint32_t trackNumber, int64_t gain);

    /**
     * Sets the name of the song.
     * 
     * @param name The new song name.
     */
    void setSongName(std::string &name) { this->name = name; }

    /**
     * Gets the length of the song in samples.
     * 
     * @return The length of the song
     */
    uint64_t getLengthInSamples();

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
     * Generates generates PCM data and append it to
     * the provided WAV file object.
     * 
     * @param file The file object to recieve data.
     */
    void generateWaveData(WaveFile &file);

    WaveFactory factory;
    std::string name;
    uint32_t sampleRate;
    Tempo tempo;
    std::deque<Track *> tracks;
};

#endif