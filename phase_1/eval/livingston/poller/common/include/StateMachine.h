#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include "Messages.h"
#include "Song.h"
#include "SystemException.h"

#define MAX_LOGINS 5

/**
 * This class describes the state machine of the server.
 * It keeps track of logins and work on songs.
 */
class StateMachine
{

public:

    /**
     * Constructs a state machine.
     */
    StateMachine();

    /**
     * Destroys a state machine
     */
    ~StateMachine();

    /**
     * Gets the client socket descriptor at the given index.
     *
     * @param index The client descriptor to get.
     * @return The requested client.
     */
    int getClient(int index);

    /**
     * Finds the entry ID of the socket descriptor.
     *
     * @param socketDescriptor The descriptor to find the id of
     * @return The id of the descriptor
     */
    int getClientNumber(int socketDescriptor);

    /**
     * Adds a client socket descriptor to the list of clients.
     *
     * @param socketDescriptor The socket descriptor to add.
     * @return True if the client socket descriptor was added.
     */
    bool insertClient(int socketDescriptor);

    /**
     * Removes a client socket descriptor to the list of clients.
     *
     * @param index The index of the socket descriptor to remove.
     * @return True if the client socket descriptor was removed.
     */
    bool removeClient(int index);

    /**
     * Processes the messages and affects the state machine.
     * The state machine will return a message that describes
     * the result of the state change.
     *
     * @param buffer The incoming message to process.
     * @param bufferSize The size of the message.
     * @param sender The client that sends the message.
     * @return The state machine reply to the message.
     */
    ControlMessage *processMessage(uint8_t *buffer, int bufferSize, int sender);

private:

    /**
     * Creates a song for the specified client. If there is
     * an existing song, it is deleted. The tempo is adjusted
     * if it is in a unacceptable range.
     *
     * @param tempo The tempo of the song to create.
     * @param clientID The id of the client
     * @return the tempo of the song created.
     */
    uint32_t createSong(uint32_t tempo, int clientID);

    /**
     * Creates a track of the given wave type and returns
     * the ID of the wave type that is in the new track. On
     * error max uint8_t is returned.
     *
     * @param waveType The ID of the wave type to create.
     * @param volume The volume of the track
     * @param lfo True if there should be an lfo
     * @param depth The depth of the LFO
     * @param detune The detune of the LFO
     * @param rate The rate of the oscilation
     * @param numTracks Returns the number of tracks created.
     * @param clientID The client to create a track for.
     * @return True if the track was created.
     */
    bool createTrack(uint8_t *waveType,
                     uint8_t *volume,
                     uint32_t *lfo,
                     uint32_t *depth,
                     uint32_t *detune,
                     uint32_t *rate,
                     uint8_t *numTracks,
                     int clientID);


     /**
      * Adds the requested not to the current track. If the parameters are not
      * within valid ranges, they are adjusted to be acceptable. The parameters
      * are reloaded with the actual values used to set the note.
      *
      * @param note The semitone
      * @param octave The octave of the tone
      * @param duration The duration of the tone
      * @param clientID The client to add the note to.
      * @return True if the add was successful.
      */
    bool addNote(uint32_t *note, uint32_t *octave, uint64_t *duration, int clientID);

    /**
     * Adds a rest to the current track.
     *
     * @param duration The duration of the rest
     * @param clientID The client to add the rest to
     * @return true if the add was successful.
     */
    bool addRest(uint64_t *duration, int clientID);

    /**
     * Sets the gain for the clients current track.
     *
     * @param gain The gain boost/cut
     * @param clientID The client to affect.
     * @return True if success.
     */
    bool setGain(int64_t *gain, int clientID);

    /**
     * Adds a delay effect to the current track. If delay
     * is 0, the beatDelay should be used.
     *
     * @param mix The mix of the effect and clean signal.
     * @param delay The delay of the new signal.
     * @param beatDelay The delay of the signal in terms of tempo.
     * @param feedback The number of times to apply delay.
     * @param priority The priority of the effect
     * @param clientID The client that has requested the effect.
     * @return
     */
    bool addDelay(uint32_t *mix, uint32_t *delay, uint32_t *beatDelay, uint32_t *feedback, uint32_t *priority, int clientID);

    /**
     * Adds a delay effect to the current track. If delay
     * is 0, the beatDelay should be used.
     *
     * @param mix The mix of the effect and clean signal.
     * @param delay The delay of the new signal.
     * @param depth The depth of the chorus in 1/10 of octave
     * @param rate The rate of the chorus in Hz
     * @param priority The priority of the effect
     * @param clientID The client that has requested the effect.
     * @return
     */
    bool addChorus(uint32_t *mix, uint32_t *delay, uint32_t *depth, uint32_t *rate, uint32_t *priority, int clientID);

    /**
     * Adds tremolo effect to the current track.
     *
     * @param mix The mix of the effect.
     * @param rate The rate of the tremolo
     * @param mode The tremolo modulation wave form.
     * @param priority The priority of the effect
     * @param clientID The client that has requested the effect.
     * @return True if successful
     */
    bool addTremolo(uint32_t *mix, uint32_t *rate, uint32_t *mode, uint32_t *priority, int clientID);

    /**
     * Adds an equalizer effect to the current track.
     *
     * @param frequency The center frequency.
     * @param gain The gain boost or cut.
     * @param bandWidth The bandwith to edit.
     * @param priority The priority of the effect.
     * @param clientID The client that has requested the effect.
     * @return True if successful
     */
    bool addEqualizerBand(uint16_t *frequency, int16_t *gain, uint16_t *bandWidth, uint32_t *priority, int clientID);

    /**
     * Adds an envelope shaper to the current track.
     *
     * @param attack The attack value of the filter.
     * @param release The release value of the filter.
     * @param priority The priority of the effect.
     * @param clientID The client that has requested the effect.
     * @return True if successful
     */
    bool addEnvelopeShaper(uint32_t *attack, uint32_t *release, uint32_t *priority, int clientID);

    /**
     * Adds a compressor to the current track.
     *
     * @param attack The attack value of the filter.
     * @param release The release value of the filter.
     * @param priority The priority of the effect.
     * @param ratio The amount of gain cut
     * @param threshold The max volume
     * @param clientID The client that has requested the effect.
     * @return True if successful
     */
    bool addCompressor(uint32_t *attack, uint32_t *release, uint32_t *ratio, uint32_t *threshold, uint32_t *priority, int clientID);

    /**
     * Removes an effect from the current track.
     *
     * @param priority The priority of the effect to remove
     * @param clientID The client requesting the removal
     * @return True on success
     */
    bool removeEffect(uint32_t *priority, int clientID);

    /**
     * Gets information about the song.
     *
     * @param numTracks The number of tracks
     * @param lenSamples The track length in samples
     * @param lenMilis The track length in millis
     * @param name The name of the song.
     * @param clientID The client making the request
     * @return True on success
     */
    bool getSongInfo(uint32_t *numTracks, uint32_t *lenSamples, uint32_t *lenMilis, const char **name, int clientID);

    /**
     * Clears the song for the client.
     *
     * @param clientNo The client number
     * @return True if successful
     */
    bool clearSong(int clientNo);

    /**
     * Sets the name of the song.
     *
     * @param name The new name
     * @param clientNo The client number
     * @return True on success.
     */
    bool setSongName(const char **name, int clientNo);

    /**
     * Bounces the song to the required stream. If indicated
     * the song will be cleared.
     *
     * @param clearSong The flag to clear the song
     * @param buffer The buffer to fill with song data
     * @param clientID The client to operate on
     * @return the size of the data buffer
     */
    size_t bounceTrack(uint64_t clearSong, uint8_t **buffer, int clientID);

    /**
     * Saves the song state.
     *
     * @param buffer The buffer
     * @param clientID The clients id
     * @return The size of the message
     */
    bool saveSongState(uint8_t **buffer, size_t *size, int clientID);

    /**
     * Sets the track to the specified track number or
     * the largest track number.
     *
     * @param track The track number to set.
     * @param clientID The client to set for.
     * @return True if the set worked.
     */
    bool setTrack(uint8_t *track, int clientID);

    /**
     * Sets the login type for the given client.
     *
     * @param loginType The login mode.
     * @param clientID The client to login
     */
    void setLogin(uint8_t loginType, int clientID);

    /**
     * Looks up the login type of the client.
     *
     * @param clientID The client to look up.
     * @return The client's login type.
     */
    uint8_t getLogin(int clientID);

    /**
     * Sets the current track for the given client.
     * If this track does not exist, nothing happens.
     *
     * @param track The track to set.
     * @param clientID The client to set.
     */
    void setCurrentTrack(uint8_t track, int clientID);

    /**
     * Gets the current track for the given client.
     *
     * @param clientID The client to get info for.
     * @return The current track set for the client.
     */
    uint8_t getCurrentTrack(int clientID);

    /**
     * Sets the number of tracks for the client.
     *
     * @param num The number of tracks for the client.
     * @param clientID The id to set.
     */
    void setNumTracks(uint8_t num, int clientID);

    /**
     * Gets the number of tracks for the client.
     *
     * @param clientID The client to get info for.
     * @return The number of tracks that the client has.
     */
    uint8_t getNumTracks(int clientID);

    /**
     * Validates the given password.
     *
     * @return true if the password is valid
     */
    bool validatePassword(const char *password);

    /**
     * This is the function that returns the flag.
     *
     * @return The flag.
     */
    const char *getFlag();

    int *clients;
    uint8_t *logins;
    uint8_t *currentTrack;
    uint8_t *numTracks;
    Song *songs[MAX_LOGINS];
    char *flag;
};

#endif
