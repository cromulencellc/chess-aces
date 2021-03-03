#ifndef MESSAGES_H
#define MESSAGES_H
#include <cstdint>
#include "Note.h"
#include "Tempo.h"
#include "WaveFactory.h"
#define LOGIN_REQUEST (0)
#define LOGIN_RESPONSE (1)
#define PASSWORD_REQUEST (2)
#define ADD_TRACK_REQUEST (3)
#define ADD_TRACK_RESPONSE (4)
#define SET_TRACK_REQUEST (5)
#define SET_TRACK_RESPONSE (6)
#define ADD_NOTE_REQUEST (7)
#define ADD_NOTE_RESPONSE (8)
#define ADD_REST_REQUEST (9)
#define ADD_REST_RESPONSE (10)
#define SET_TRACK_GAIN_REQUEST (11)
#define SET_TRACK_GAIN_RESPONSE (12)
#define ADD_DELAY_REQUEST (13)
#define ADD_DELAY_RESPONSE (14)
#define ADD_TREMOLO_REQUEST (15)
#define ADD_TREMOLO_RESPONSE (16)
#define ADD_CHORUS_REQUEST (17)
#define ADD_CHORUS_RESPONSE (18)
#define ADD_EQ_BAND_REQUEST (19)
#define ADD_EQ_BAND_RESPONSE (20)
#define REMOVE_EFFECT_REQUEST (21)
#define REMOVE_EFFECT_RESPONSE (22)
#define CREATE_SONG_REQUEST (23)
#define CREATE_SONG_RESPONSE (24)
#define BOUNCE_SONG_REQUEST (25)
#define BOUNCE_SONG_RESPONSE (26)
#define SHUTDOWN_REQUEST (27)
#define SHUTDOWN_RESPONSE (28)
#define SPECIAL_KEY_REQUEST (29)
#define SPECIAL_KEY_RESPONSE (30)
#define SONG_INFO_REQUEST (31)
#define SONG_INFO_RESPONSE (32)
#define CLEAR_SONG_REQUEST (33)
#define CLEAR_SONG_RESPONSE (34)
#define SET_SONG_NAME_REQUEST (35)
#define SET_SONG_NAME_RESPONSE (36)
#define SAVE_STATE_REQUEST (37)
#define SAVE_STATE_RESPONSE (38)
#define ERROR_RESPONSE (39)
#define LOGIN_NORMAL (0)
#define LOGIN_ADMIN (1)
#define MALFORMED_MESSAGE (0)
#define UNKNOWN_MESSAGE (1)
#define INVALID_LOGIN (2)
#define INVALID_PASSWORD (3)
#define INVALID_BOUNCE (4)
#define INVALID_TRACK_ADD (5)
#define INVALID_TRACK_SET (6)
#define INVALID_TRACK_ACTION (7)
#define INVALID_NOTE_ADD (8)
#define INCALID_EFFECT_ADD (9)
#define INVALID_SHUTDOWN (10)
#define INVALID_REQUEST_FOR_FLAG (11)
#define INVALID_SONG_ACTION (12)
#define NO (0)
#define YES (1)
typedef struct ControlMessage {
  uint32_t messageSize;
  uint32_t messageType;
} ControlMessage;
typedef struct LoginReqestMessage { uint64_t loginRequest; } LoginReqestMessage;
typedef struct LoginResponseMessage {
  uint64_t loginResponse;
} LoginResponseMessage;
typedef struct PasswordValidationRequest {
  uint8_t password[128];
} PasswordValidationRequest;
typedef struct AddTrackRequest {
  uint32_t waveType;
  uint32_t volume;
  uint32_t lfo;
  uint32_t depth;
  uint32_t detune;
  uint32_t rate;
} AddTrackRequest;
typedef struct AddTrackResponse {
  uint32_t numTracks;
  uint32_t waveType;
  uint32_t volume;
  uint32_t lfo;
  uint32_t depth;
  uint32_t detune;
  uint64_t rate;
} AddTrackResponse;
typedef struct SetTrackRequest { uint64_t trackNumber; } SetTrackRequest;
typedef struct SetTrackResponse { uint64_t trackNumber; } SetTrackResponse;
typedef struct AddNoteRequest {
  uint32_t note;
  uint32_t octave;
  uint64_t duration;
} AddNoteRequest;
typedef struct AddNoteResponse {
  uint32_t note;
  uint32_t octave;
  uint64_t duration;
} AddNoteResponse;
typedef struct AddRestRequest { uint64_t duration; } AddRestRequest;
typedef struct AddRestResponse { uint64_t duration; } AddRestResponse;
typedef struct SetTrackGainRequest { int64_t gain; } SetTrackGainRequest;
typedef struct SetTrackGainResponse { int64_t gain; } SetTrackGainResponse;
typedef struct AddDelayRequest {
  uint32_t mix;
  uint32_t delay;
  uint32_t beatDelay;
  uint32_t feedback;
  uint64_t priority;
} AddDelayRequest;
typedef struct AddDelayResponse {
  uint32_t mix;
  uint32_t delay;
  uint32_t beatDelay;
  uint32_t feedback;
  uint64_t priority;
} AddDelayResponse;
typedef struct AddChorusRequest {
  uint32_t mix;
  uint32_t delay;
  uint32_t depth;
  uint32_t rate;
  uint64_t priority;
} AddChorusRequest;
typedef struct AddChorusResponse {
  uint32_t mix;
  uint32_t delay;
  uint32_t depth;
  uint32_t rate;
  uint64_t priority;
} AddChorusResponse;
typedef struct AddTremoloRequest {
  uint32_t mix;
  uint32_t rate;
  uint32_t mode;
  uint32_t priority;
} AddTremoloRequest;
typedef struct AddTremoloResponse {
  uint32_t mix;
  uint32_t rate;
  uint32_t mode;
  uint32_t priority;
} AddTremoloResponse;
typedef struct AddEqBandRequest {
  uint16_t frequency;
  int16_t gain;
  uint32_t bandwidth;
  uint64_t priority;
} AddEqBandRequest;
typedef struct AddEqBandResponse {
  uint16_t frequency;
  int16_t gain;
  uint32_t bandwidth;
  uint64_t priority;
} AddEqBandResponse;
typedef struct RemoveEffectRequest { uint64_t priority; } RemoveEffectRequest;
typedef struct RemoveEffectResponse { uint64_t priority; } RemoveEffectResponse;
typedef struct CreateSongRequest { uint64_t tempo; } CreateSongRequest;
typedef struct CreateSongResponse { uint64_t tempo; } CreateSongResponse;
typedef struct BounceSongRequest { uint64_t clearSong; } BounceSongRequest;
typedef struct BounceSongResponse {
  uint64_t size;
  uint8_t data[1];
} BounceSongResponse;
typedef struct SaveStateResponse {
  uint64_t size;
  uint8_t data[1];
} SaveStateResponse;
typedef struct SpecialKeyResponse { uint8_t key[128]; } SpecialKeyResponse;
typedef struct ErrorResponse { uint64_t errorCode; } ErrorResponse;
typedef struct SongInfoResponse {
  uint32_t numTracks;
  uint32_t lenSamples;
  uint64_t lenMillis;
  uint8_t name[64];
} SongInfoResponse;
typedef struct SetSongNameRequest { uint8_t name[64]; } SetSongNameRequest;
typedef struct SetSongNameResponse { uint8_t name[64]; } SetSongNameResponse;
ControlMessage *makeLoginReqestMessage(uint64_t loginRequest);
ControlMessage *makeLoginResponseMessage(uint64_t loginResponse);
ControlMessage *makePasswordValidationRequest(const char *passwd);
ControlMessage *makeAddTrackRequest(WaveFactory::WaveType wave, uint32_t volume,
                                    uint32_t lfo, uint32_t depth,
                                    uint32_t detune, uint32_t rate);
ControlMessage *makeAddTrackResponse(uint32_t numTracks,
                                     WaveFactory::WaveType wave,
                                     uint32_t volume, uint32_t lfo,
                                     uint32_t depth, uint32_t detune,
                                     uint32_t rate);
ControlMessage *makeSetTrackRequest(uint64_t trackNumber);
ControlMessage *makeSetTrackResponse(uint64_t trackNumber);
ControlMessage *makeAddNoteRequest(Semitone tone, int octave, Tempo::Beat beat);
ControlMessage *makeAddNoteResponse(Semitone tone, int octave,
                                    Tempo::Beat beat);
ControlMessage *makeAddRestRequest(Tempo::Beat beat);
ControlMessage *makeAddRestResponse(Tempo::Beat beat);
ControlMessage *makeSetTrackGainRequest(int64_t gain);
ControlMessage *makeSetTrackGainResponse(int64_t gain);
ControlMessage *makeAddDelayRequest(uint32_t mix, uint32_t delay,
                                    Tempo::Beat beatDelay, uint32_t feedback,
                                    uint64_t priority);
ControlMessage *makeAddDelayResponse(uint32_t mix, uint32_t delay,
                                     Tempo::Beat beatDelay, uint32_t feedback,
                                     uint64_t priority);
ControlMessage *makeAddChorusRequest(uint32_t mix, uint32_t delay,
                                     uint32_t depth, uint32_t rate,
                                     uint32_t priority);
ControlMessage *makeAddChorusResponse(uint32_t mix, uint32_t delay,
                                      uint32_t depth, uint32_t rate,
                                      uint32_t priority);
ControlMessage *makeAddTremoloRequest(uint32_t mix, uint32_t rate,
                                      uint32_t mode, uint64_t priority);
ControlMessage *makeAddTremoloResponse(uint32_t mix, uint32_t rate,
                                       uint32_t mode, uint64_t priority);
ControlMessage *makeAddEqBandRequest(uint16_t frequency, int16_t gain,
                                     uint32_t bandwidth, uint64_t priority);
ControlMessage *makeAddEqBandResponse(uint16_t frequency, int16_t gain,
                                      uint32_t bandwidth, uint64_t priority);
ControlMessage *makeRemoveEffectRequest(uint32_t priority);
ControlMessage *makeRemoveEffectResponse(uint32_t priority);
ControlMessage *makeCreateSongRequest(uint64_t tempo);
ControlMessage *makeCreateSongResponse(uint64_t tempo);
ControlMessage *makeBounceSongRequest(uint64_t clearSong);
ControlMessage *makeBounceSongResponse(uint64_t size, uint8_t *data);
ControlMessage *makeSaveStateRequest();
ControlMessage *makeSaveStateResponse(uint64_t size, uint8_t *data);
ControlMessage *makeShutdownRequest();
ControlMessage *makeShutdownResponse();
ControlMessage *makeSpecialKeyRequest();
ControlMessage *makeSpecialKeyResponse(const char *key);
ControlMessage *makeErrorResponse(uint64_t errorCode);
ControlMessage *makeSongInfoRequest();
ControlMessage *makeSongInfoResponse(uint32_t numTracks, uint32_t lenSamples,
                                     uint64_t lenMillis, const char *name);
ControlMessage *makeClearSongRequest();
ControlMessage *makeClearSongResponse();
ControlMessage *makeSetNameRequest(const char *name);
ControlMessage *makeSetNameResponse(const char *name);
ControlMessage *parseMessage(uint8_t *message, int bufferSize);
#endif