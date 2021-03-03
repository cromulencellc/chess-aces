#ifndef MESSAGES_H
#define MESSAGES_H

#include <cstdint>

#include "WaveFactory.h"
#include "Note.h"
#include "Tempo.h"

/**
 * Message types
 */
#define LOGIN_REQUEST            (0)
#define LOGIN_RESPONSE           (1)
#define PASSWORD_REQUEST         (2)
#define ADD_TRACK_REQUEST        (3)
#define ADD_TRACK_RESPONSE       (4)
#define SET_TRACK_REQUEST        (5)
#define SET_TRACK_RESPONSE       (6)
#define ADD_NOTE_REQUEST         (7)
#define ADD_NOTE_RESPONSE        (8)
#define ADD_REST_REQUEST         (9)
#define ADD_REST_RESPONSE        (10)
#define SET_TRACK_GAIN_REQUEST   (11)
#define SET_TRACK_GAIN_RESPONSE  (12)
#define ADD_DELAY_REQUEST        (13)
#define ADD_DELAY_RESPONSE       (14)
#define ADD_TREMOLO_REQUEST      (15)
#define ADD_TREMOLO_RESPONSE     (16)
#define ADD_CHORUS_REQUEST       (17)
#define ADD_CHORUS_RESPONSE      (18)
#define ADD_EQ_BAND_REQUEST      (19)
#define ADD_EQ_BAND_RESPONSE     (20)
#define REMOVE_EFFECT_REQUEST    (21)
#define REMOVE_EFFECT_RESPONSE   (22)
#define CREATE_SONG_REQUEST      (23)
#define CREATE_SONG_RESPONSE     (24)
#define BOUNCE_SONG_REQUEST      (25)
#define BOUNCE_SONG_RESPONSE     (26)
#define SHUTDOWN_REQUEST         (27)
#define SHUTDOWN_RESPONSE        (28)
#define SPECIAL_KEY_REQUEST      (29)
#define SPECIAL_KEY_RESPONSE     (30)
#define SONG_INFO_REQUEST        (31)
#define SONG_INFO_RESPONSE       (32)
#define CLEAR_SONG_REQUEST       (33)
#define CLEAR_SONG_RESPONSE      (34)
#define SET_SONG_NAME_REQUEST    (35)
#define SET_SONG_NAME_RESPONSE   (36)
#define SAVE_STATE_REQUEST       (37)
#define SAVE_STATE_RESPONSE      (38)
#define ERROR_RESPONSE           (39)

/**
 * Login states
 */
#define LOGIN_NORMAL     (0)
#define LOGIN_ADMIN      (1)

/**
 * Error Codes
 */
#define MALFORMED_MESSAGE         (0)
#define UNKNOWN_MESSAGE           (1)
#define INVALID_LOGIN             (2)
#define INVALID_PASSWORD          (3)
#define INVALID_BOUNCE            (4)
#define INVALID_TRACK_ADD         (5)
#define INVALID_TRACK_SET         (6)
#define INVALID_TRACK_ACTION      (7)
#define INVALID_NOTE_ADD          (8)
#define INCALID_EFFECT_ADD        (9)
#define INVALID_SHUTDOWN         (10)
#define INVALID_REQUEST_FOR_FLAG (11)
#define INVALID_SONG_ACTION      (12)

/**
 * Answers
 */
#define NO  (0)
#define YES (1)

/**
 * The message header for any control message.
 * 
 * @param messageSize  The message size.
 * @param messageType  The message type.
 */
typedef struct ControlMessage {
    uint32_t messageSize;
    uint32_t messageType;
} ControlMessage;

/**
 * A request for a login.
 * 
 * @param loginRequest  The login type. 
 */
typedef struct LoginReqestMessage {
    uint64_t loginRequest;
} LoginReqestMessage;

/**
 * A response for a login.
 * 
 * @param loginResponse  New login state. 
 */
typedef struct LoginResponseMessage {
    uint64_t loginResponse;
} LoginResponseMessage;

/**
 * A password validation request.
 * 
 * @param password  The password to validate. 
 */
typedef struct PasswordValidationRequest {
    uint8_t password[128];
} PasswordValidationRequest;

/**
 * A request to add a track with a given wave 
 * form associated with it. The current track
 * number is set to this track.
 * 
 * @param waveType  The type of wave that the track will generate
 * @param volume The volume of the new track (1 - 100)
 * @param lfo True if there should be an LFO (0 - 1)
 * @param depth The depth of the LFO in 1/10 of octaves
 * @param detune in cents (1/100th of a step)
 * @param rate The rate of oscilation in Hz
 */
typedef struct AddTrackRequest {
    uint32_t waveType;
    uint32_t volume;
    uint32_t lfo;
    uint32_t depth;
    uint32_t detune;
    uint32_t rate;
} AddTrackRequest;

/**
 * A response to the add track request. 
 * 
 * @param numTracks  The number of tracks on the song.
 * @param waveType  The type of wave that the track will generate
 * @param volume The volume of the new track (1 - 100)
 * @param lfo True if there should be an LFO (0 - 1)
 * @param depth The depth of the LFO in 1/10 of octaves
 * @param detune in cents (1/100th of a step)
 * @param rate The rate of oscilation in Hz
 */
typedef struct AddTrackResponse {
    uint32_t numTracks;
    uint32_t waveType;
    uint32_t volume;
    uint32_t lfo;
    uint32_t depth;
    uint32_t detune; 
    uint64_t rate;   
} AddTrackResponse;

/**
 * A request to set the track to edit.
 * 
 * @param trackNumber  The track to edit.
 */
typedef struct SetTrackRequest {
    uint64_t trackNumber;
} SetTrackRequest;

/**
 * A response to the request to set the 
 * track to edit.
 * 
 * @param trackNumber  The track to edit.
 */
typedef struct SetTrackResponse {
    uint64_t trackNumber;
} SetTrackResponse;

/**
 * A request to add a note to the current
 * track being edited.
 * 
 * @param note  The note to add.
 * @param octave The octave of the note to add.
 * @param duration The duration of the note to add.
 */
typedef struct AddNoteRequest {
    uint32_t note;
    uint32_t octave;
    uint64_t duration;
} AddNoteRequest;

/**
 * A response to the request to add a note to the current
 * track being edited.
 * 
 * @param note  The note to add.
 * @param octave The octave of the note to add.
 * @param duration The duration of the note to add.
 */
typedef struct AddNoteResponse {
    uint32_t note;
    uint32_t octave;
    uint64_t duration;
} AddNoteResponse;

/**
 * A request to add a rest to the track.
 * 
 * @param duration The duration of the rest.
 */
typedef struct AddRestRequest {
    uint64_t duration;
} AddRestRequest;

/**
 * A response to the request to add a rest to the track.
 * 
 * @param duration The duration of the rest.
 */
typedef struct AddRestResponse {
    uint64_t duration;
} AddRestResponse;

/**
 * Requests to set the gain for
 * the current track. Gain is in dB.
 * 
 * @param gain The gain boost/cut
 */
typedef struct SetTrackGainRequest {
    int64_t gain;
} SetTrackGainRequest;

/**
 * Responds to the request to set the gain for
 * the current track. Gain is in dB.
 * 
 * @param gain The gain boost/cut
 */
typedef struct SetTrackGainResponse {
    int64_t gain;
} SetTrackGainResponse;

/**
 * A request to add delay to a track. If delay is set
 * to 0, then beatDelay is used and the delay will be in
 * tempo. The effect is applied to the current track.
 * 
 * @param mix The mix of the effect and clean signal.
 * @param delay The delay of the new signal.
 * @param beatDelay The delay of the signal in terms of tempo.
 * @param feedback The number of times to apply delay.
 * @param priority The priority of the effect.
 */
typedef struct AddDelayRequest {
    uint32_t mix;
    uint32_t delay;
    uint32_t beatDelay;
    uint32_t feedback;
    uint64_t priority;
} AddDelayRequest;

/**
 * A response to the request to add delay to a track.
 * 
 * @param mix The mix of the effect and clean signal.
 * @param delay The delay of the new signal.
 * @param beatDelay The delay of the signal in terms of tempo.
 * @param feedback The number of times to apply delay.
 * @param priority The priority of the effect.
 */
typedef struct AddDelayResponse {
    uint32_t mix;
    uint32_t delay;
    uint32_t beatDelay;
    uint32_t feedback;
    uint64_t priority;
} AddDelayResponse;

/**
 * A request to add chorus to a track.  The effect is applied 
 * to the current track.
 * 
 * @param mix The mix of the effect and clean signal.
 * @param delay The offset of the mixed signal in ms.
 * @param depth The depth of the chorus.
 * @param rate The rate of the chorus.
 * @param priority The priority of the effect.
 */
typedef struct AddChorusRequest {
    uint32_t mix;
    uint32_t delay;
    uint32_t depth;
    uint32_t rate;
    uint64_t priority;
} AddChorusRequest;

/**
 * A response to the request to add chorus 
 * to a track.
 * 
 * @param mix The mix of the effect and clean signal.
 * @param delay The offset of the mixed signal in ms.
 * @param depth The depth of the chorus.
 * @param rate The rate of the chorus.
 * @param priority The priority of the effect.
 */
typedef struct AddChorusResponse {
    uint32_t mix;
    uint32_t delay;
    uint32_t depth;
    uint32_t rate;
    uint64_t priority;
} AddChorusResponse;

/**
 * A request to add tremolo to the track.
 * 
 * @param mix The mix of wet signal.
 * @param rate The rate of the tremolo.
 * @param mode The waveform of the volume modulation 
 *             0 Sine 
 *             1 Square
 *             2 Triangle 
 *             3 Saw
 * @param priority The priority of the effect.
 */
typedef struct AddTremoloRequest {
    uint32_t mix;
    uint32_t rate;
    uint32_t mode;
    uint32_t priority;
} AddTremoloRequest;

/**
 * A request to add tremolo to the track.
 * 
 * @param mix The mix of wet signal.
 * @param rate The rate of the tremolo.
 * @param mode The waveform of the volume modulation 
 *             0 Sine 
 *             1 Square
 *             2 Triangle 
 *             3 Saw 
 * @param priority The priority of the effect.
 */
typedef struct AddTremoloResponse {
    uint32_t mix;
    uint32_t rate;
    uint32_t mode;
    uint32_t priority;
} AddTremoloResponse;

/**
 * Adds an eq band to the track. The eq acts like an
 * effect.
 * 
 * @param frequency The frequency to effect.
 * @param gain The gain boost or cut.
 * @param bandwidth The bandwidth of the boost/cut.
 * @param priority The priority in the effect.
 */
typedef struct AddEqBandRequest {
    uint16_t frequency;
    int16_t  gain;
    uint32_t bandwidth;
    uint64_t priority;
} AddEqBandRequest;

/**
 * Response to the request to add an eq band to the track. 
 * 
 * @param frequency The frequency to effect.
 * @param gain The gain boost or cut.
 * @param bandwidth The bandwidth of the boost/cut.
 * @param priority The priority in the effect.
 */
typedef struct AddEqBandResponse {
    uint16_t frequency;
    int16_t  gain;
    uint32_t bandwidth;
    uint64_t priority;
} AddEqBandResponse;

/**
 * Request to remove an effect from thr current track.
 * 
 * @param priotiry The priority of the track to remove
 */
typedef struct RemoveEffectRequest {
    uint64_t priority;
} RemoveEffectRequest;

/**
 * Response to the request to remove an effect from thr current track.
 * 
 * @param priotiry The priority of the track to remove
 */
typedef struct RemoveEffectResponse {
    uint64_t priority;
} RemoveEffectResponse;

/**
 * Creates request to create a song for 
 * the given session.
 * 
 * @param tempo The tempo of the session.
 */
typedef struct CreateSongRequest {
    uint64_t tempo;
} CreateSongRequest;

/**
 * Creates a response to the request to
 * create a song for the given user.
 * 
 * @param tempo created.
 */
typedef struct CreateSongResponse {
    uint64_t tempo;
} CreateSongResponse;

/**
 * Creates a request to bounce the song.
 * 
 * @param clearSong Determines if the song should be cleared.
 */
typedef struct BounceSongRequest {
    uint64_t clearSong;
} BounceSongRequest;

/**
 * Response containing the bounced file data.
 * 
 * @param size The size of the data
 * @param data The data
 */
typedef struct BounceSongResponse {
    uint64_t size;
    uint8_t data[1];
} BounceSongResponse; 

/**
 * Responds to the request to save state.
 * 
 * @param size The size of the data.
 * @param data The data
 */
typedef struct SaveStateResponse {
    uint64_t size; 
    uint8_t data[1];
} SaveStateResponse;

/**
 * The message that returns the flag.
 * 
 * @param key The flag 
 */
typedef struct SpecialKeyResponse {
    uint8_t key[128];
} SpecialKeyResponse;

/**
 * Creates a response that reports
 * an error.
 * 
 * @param errorCode The error code.
 */
typedef struct ErrorResponse {
    uint64_t errorCode;
} ErrorResponse;

/**
 * Responds to the request for song information.
 * 
 * @param numTracks The number of tracks in the song.
 * @param lenSamples The length of the song in samples.
 * @param lenMillis The length of the song in milli.
 * @param name The name of the song.
 */
typedef struct SongInfoResponse {
    uint32_t numTracks;
    uint32_t lenSamples;
    uint64_t lenMillis;
    uint8_t name[64];
} SongInfoResponse;

/**
 * The request to set the name of the song.
 * 
 * @param name
 */
typedef struct SetSongNameRequest {
    uint8_t name[64];
} SetSongNameRequest;

/**
 * The response to the request to set 
 * the song name.
 * 
 * @param name The new name
 */
typedef struct SetSongNameResponse {
    uint8_t name[64];
} SetSongNameResponse;

/**
 * Creates a login request message. 
 * 
 * @param loginRequest The type of login requested.
 * @return The message
 *  */
ControlMessage *makeLoginReqestMessage(uint64_t loginRequest);

/**
 * Creates a login response message.
 * 
 * @param loginResponse The new login state.
 * @return The message
 */
ControlMessage *makeLoginResponseMessage(uint64_t loginResponse);

/**
 * Creates a request to validate a password.
 * 
 * @param passwd The password to validate.
 * @return The message
 */
ControlMessage *makePasswordValidationRequest(const char *passwd);

/**
 * Creates a request to add a new track.
 * 
 * @param wave The wave form of the new track.
 * @param volume The volume of the new track.
 * @param lfo True if there should be an LFO (0 - 1)
 * @param depth The depth of the LFO (1 - 100)
 * @param detune in cents (1/100th of a step)
 * @param rate The rate of oscilation in Hz
 * @return The message
 */
ControlMessage *makeAddTrackRequest(WaveFactory::WaveType wave, 
                                    uint32_t volume, 
                                    uint32_t lfo, 
                                    uint32_t depth, 
                                    uint32_t detune,
                                    uint32_t rate);

/**
 * Creates a response to a request to add a track.
 * 
 * @param numTracks The track number that was added.
 * @param wave The wave form of the new track.
 * @param volume The volume of the new track.
 * @param lfo True if there should be an LFO (0 - 1)
 * @param depth The depth of the LFO in 10ths of octives
 * @param detune in cents (1/100th of a step)
 * @param rate The rate of oscilation in Hz
 * @return The message
 */
ControlMessage *makeAddTrackResponse(uint32_t numTracks, 
                                     WaveFactory::WaveType wave, 
                                     uint32_t volume, 
                                     uint32_t lfo, 
                                     uint32_t depth, 
                                     uint32_t detune,
                                     uint32_t rate);

/**
 * Creates a request to set the track being edited.
 * 
 * @param trackNumber The track number to edit.
 * @return The message
 */
ControlMessage *makeSetTrackRequest(uint64_t trackNumber);

/**
 * Creates a response to the request to set the current track.
 * 
 * @param trackNumber The track nuber that is being editied.
 * @return The message
 */
ControlMessage *makeSetTrackResponse(uint64_t trackNumber);

/**
 * Creates a request to add a note to the current track.
 * 
 * @param tone The tone of the note to add.
 * @param octave The octave of the tone to add.
 * @param beat The beat of the tone to add.
 * @return The message
 */
ControlMessage *makeAddNoteRequest(Semitone tone, int octave, Tempo::Beat beat);

/**
 * Creates the response to the request to add a note to the current track.
 * 
 * @param tone The tone of the added note.
 * @param octave The octave of the added note.
 * @param beat The beat of the added note.
 * @return The message
 */
ControlMessage *makeAddNoteResponse(Semitone tone, int octave, Tempo::Beat beat);

/**
 * Creates a request to add a rest to the current track.
 * 
 * @param beat The beat to set the rest to.
 * @return The message
 */
ControlMessage *makeAddRestRequest(Tempo::Beat beat);

/**
 * Creates a response to the request to add a rest to the
 * current track.
 * 
 * @param beat The beat that the rest was set to.
 * @return The message
 */
ControlMessage *makeAddRestResponse(Tempo::Beat beat);

/**
 * Creates a request to set the gain for the current track.
 * 
 * @param gain The gain boost or cut
 * @return The message
 */
ControlMessage *makeSetTrackGainRequest(int64_t gain);

/**
 * Creates a response to the request to set the gain for the current track.
 * 
 * @param gain The gain boost or cut
 * @return The message
 */
ControlMessage *makeSetTrackGainResponse(int64_t gain);

/**
 * Creates a request to add delay to a track. If delay is 0, then beatDelay is used 
 * to determine the delay interval, which will lock the delay to the tempo.  The effect is 
 * applied to the current track.
 * 
 * @param mix The mix of the effect and clean signal.
 * @param delay The delay of the new signal.
 * @param beatDelay The delay of the signal in terms of tempo.
 * @param feedback The number of times to apply delay.
 * @param priority The priority of the effect.  
 * @return The message.
 */
ControlMessage *makeAddDelayRequest(uint32_t mix, uint32_t delay, Tempo::Beat beatDelay, uint32_t feedback, uint64_t priority);

/**
 * Creates a response to the request to add delay to a track. 
 * 
 * @param mix The mix of the effect and clean signal.
 * @param delay The delay of the new signal.
 * @param beatDelay The delay of the signal in terms of tempo.
 * @param feedback The number of times to apply delay.
 * @param priority The priority of the effect.
 * @return The message.
 */
ControlMessage *makeAddDelayResponse(uint32_t mix, uint32_t delay, Tempo::Beat beatDelay, uint32_t feedback, uint64_t priority);

/**
 * Creates a request to add chorus to the track.  The effect is applied to the current track.
 * 
 * @param mix The mix of the effect and clean signal.
 * @param delay The offset of the mixed signal in ms.
 * @param depth The depth of the chorus.
 * @param rate The rate of the chorus in Hz.
 * @param priority The priority of the effect.
 * @return The message.
 */
ControlMessage *makeAddChorusRequest(uint32_t mix, uint32_t delay, uint32_t depth, uint32_t rate, uint32_t priority);

/**
 * Creates a response to the request to add delay to the track.
 * 
 * @param mix The mix of the effect and clean signal.
 * @param delay The offset of the mixed signal in ms.
 * @param depth The depth of the chorus.
 * @param the rate of the chorus in Hz.
 * @param priority The priority of the effect.
 * @return The message.
 */
ControlMessage *makeAddChorusResponse(uint32_t mix, uint32_t delay, uint32_t depth, uint32_t rate, uint32_t priority);

/**
 * Creates a request to add tremolo to a track.
 * 
 * @param mix The mix of the effect.
 * @param rate The rate of the tremolo.
 * @param mode The waveform of the volume modulation 
 *             0 Sine 
 *             1 Square
 *             2 Triangle 
 *             3 Saw 
 * @param priority The priority of the effect.
 * @return The message
 */
ControlMessage *makeAddTremoloRequest(uint32_t mix, uint32_t rate, uint32_t mode, uint64_t priority);

/**
 * Creates a response to the request to add tremolo to a track.
 * 
 * @param mix The mix of the effect.
 * @param rate The rate of the tremolo
 * @param mode The waveform of the volume modulation 
 *             0 Sine 
 *             1 Square
 *             2 Triangle 
 *             3 Saw 
 * @param priority The priority of the effect.
 * @return The message
 */
ControlMessage *makeAddTremoloResponse(uint32_t mix, uint32_t rate, uint32_t mode, uint64_t priority);

/**
 * Makes a request to add an EQ band to the track.
 * 
 * @param frequency The frequency to adjust.
 * @param gain The gain boost or cut.
 * @param bandwidth The bandwidth around the frequency.
 * @param priority The priority of the EQ.
 * @return The message
 */
ControlMessage *makeAddEqBandRequest(uint16_t frequency, int16_t  gain, uint32_t bandwidth, uint64_t priority);

/**
 * Makes a response to the request to add an EQ band to the track.
 * 
 * @param frequency The frequency to adjust.
 * @param gain The gain boost or cut.
 * @param bandwidth The bandwidth around the frequency.
 * @param priority The priority of the EQ.
 * @return The message
 */
ControlMessage *makeAddEqBandResponse(uint16_t frequency, int16_t  gain, uint32_t bandwidth, uint64_t priority);

/**
 * Makes a request to remove an effect.
 * 
 * @param priority The priority of the effect to remove.
 */
ControlMessage *makeRemoveEffectRequest(uint32_t priority);

/**
 * Makes a response to the request to remove an effect.
 * 
 * @param priority The priority of the effect to remove.
 */
ControlMessage *makeRemoveEffectResponse(uint32_t priority);

/**
 * Creates a request to create a song.
 * 
 * @param tempo The tempo of the song to create
 * @return The message
 */
ControlMessage *makeCreateSongRequest(uint64_t tempo);

/**
 * Responds for the request to create a song.
 * 
 * @param tempo The tempo of the song created
 * @return The message
 */
ControlMessage *makeCreateSongResponse(uint64_t tempo);

/**
 * Requests the request to bounce the song.
 * 
 * @param clearSong Clears the song after bounce
 * @return The message
 */
ControlMessage *makeBounceSongRequest(uint64_t clearSong);

/**
 * Responds to the request for song data.
 * 
 * @param size The size of the data stream
 * @param data The data stream
 * @return The message
 */
ControlMessage *makeBounceSongResponse(uint64_t size, uint8_t *data);

/**
 * Creates a request for a save state file.
 * 
 * @return The message
 */
ControlMessage *makeSaveStateRequest();

/**
 * Creates the response to the request for save state
 * data.
 * 
 * @param size The size of the data
 * @param data The data
 * @return  The message
 */
ControlMessage *makeSaveStateResponse(uint64_t size, uint8_t *data);

/**
 * Requests that the server shutdown.
 * 
 * @return A shutdown request
 */
ControlMessage *makeShutdownRequest();

/**
 * Responds to the server shutdown.
 * 
 * @return A shutdown response
 */
ControlMessage *makeShutdownResponse();

/**
 * Creates the message that requests thet
 * special key.
 * 
 * @reaturn a key request.
 */
ControlMessage *makeSpecialKeyRequest();

/**
 * Creates the message that returns the key.
 * 
 * @param key The key to set.
 * @param a key response.
 */
ControlMessage *makeSpecialKeyResponse(const char *key);

/**
 * Creates an error response message.
 * 
 * @param errorCode The error code.
 * @return The message
 */
ControlMessage *makeErrorResponse(uint64_t errorCode);

/**
 * Requests for information about the song.
 * 
 * @return The message
 */
ControlMessage *makeSongInfoRequest();

/**
 * Response to the request for song information.
 * 
 * @param numTracks The number of tracks
 * @return The message
 */
ControlMessage *makeSongInfoResponse(uint32_t numTracks, uint32_t lenSamples, uint64_t lenMillis, const char *name);

/**
 * Requests to clear the song.
 * 
 * @return The message
 */
ControlMessage *makeClearSongRequest();

/**
 * Responds to the request to clear the song.
 * 
 * @return The message
 */
ControlMessage *makeClearSongResponse();

/**
 * Requests to set the name of the song.
 * 
 * @param name The name of the song
 * @return The message
 */
ControlMessage *makeSetNameRequest(const char* name);

/**
 * The response to the request to set the name of the song.
 * 
 * @param name The name of the song.
 * @return The message.
 */
ControlMessage *makeSetNameResponse(const char* name);

/**
 * Parses a control message from a buffer.
 * 
 * @param message The message to parse.
 * @param bufferSize The size of the buffer to parse the message from.
 * @return The parsed message
 */
ControlMessage *parseMessage(uint8_t *message, int bufferSize);

#endif