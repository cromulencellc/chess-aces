#include <cstdlib>
#include <cstring>
#include <iostream>
#include "Messages.h"
static ControlMessage *newMessage(uint32_t extentionSize,
                                  uint32_t messageType) {
  uint32_t totalMessageSize = extentionSize + sizeof(ControlMessage);
  ControlMessage *message = (ControlMessage *)malloc(totalMessageSize);
  if (message) {
    memset(message, 0, totalMessageSize);
    message->messageSize = totalMessageSize;
    message->messageType = messageType;
  }
  return message;
}
ControlMessage *makeLoginReqestMessage(uint64_t loginRequest) {
  ControlMessage *retVal =
      newMessage(sizeof(LoginReqestMessage), LOGIN_REQUEST);
  if (retVal) {
    LoginReqestMessage *request = (LoginReqestMessage *)(retVal + 1);
    request->loginRequest = loginRequest;
  }
  return retVal;
}
ControlMessage *makeLoginResponseMessage(uint64_t loginResponse) {
  ControlMessage *retVal =
      newMessage(sizeof(LoginResponseMessage), LOGIN_RESPONSE);
  if (retVal) {
    LoginResponseMessage *response = (LoginResponseMessage *)(retVal + 1);
    response->loginResponse = loginResponse;
  }
  return retVal;
}
ControlMessage *makePasswordValidationRequest(const char *passwd) {
  ControlMessage *retVal =
      newMessage(sizeof(PasswordValidationRequest), PASSWORD_REQUEST);
  if (retVal) {
    PasswordValidationRequest *request =
        (PasswordValidationRequest *)(retVal + 1);
    ::snprintf((char *)request->password, sizeof(request->password) - 1, "%s",
               passwd);
  }
  return retVal;
}
ControlMessage *makeAddTrackRequest(WaveFactory::WaveType wave, uint32_t volume,
                                    uint32_t lfo, uint32_t depth,
                                    uint32_t detune, uint32_t rate) {
  ControlMessage *retVal =
      newMessage(sizeof(AddTrackRequest), ADD_TRACK_REQUEST);
  if (retVal) {
    AddTrackRequest *request = (AddTrackRequest *)(retVal + 1);
    request->waveType = wave;
    request->volume = volume;
    request->lfo = lfo;
    request->depth = depth;
    request->detune = detune;
    request->rate = rate;
  }
  return retVal;
}
ControlMessage *makeAddTrackResponse(uint32_t numTracks,
                                     WaveFactory::WaveType wave,
                                     uint32_t volume, uint32_t lfo,
                                     uint32_t depth, uint32_t detune,
                                     uint32_t rate) {
  ControlMessage *retVal =
      newMessage(sizeof(AddTrackResponse), ADD_TRACK_RESPONSE);
  if (retVal) {
    AddTrackResponse *response = (AddTrackResponse *)(retVal + 1);
    response->numTracks = numTracks;
    response->waveType = wave;
    response->volume = volume;
    response->lfo = lfo;
    response->depth = depth;
    response->detune = detune;
    response->rate = rate;
  }
  return retVal;
}
ControlMessage *makeSetTrackRequest(uint64_t trackNumber) {
  ControlMessage *retVal =
      newMessage(sizeof(SetTrackRequest), SET_TRACK_REQUEST);
  if (retVal) {
    SetTrackRequest *request = (SetTrackRequest *)(retVal + 1);
    request->trackNumber = trackNumber;
  }
  return retVal;
}
ControlMessage *makeSetTrackResponse(uint64_t trackNumber) {
  ControlMessage *retVal =
      newMessage(sizeof(SetTrackResponse), SET_TRACK_RESPONSE);
  if (retVal) {
    SetTrackResponse *response = (SetTrackResponse *)(retVal + 1);
    response->trackNumber = trackNumber;
  }
  return retVal;
}
ControlMessage *makeAddNoteRequest(Semitone tone, int octave,
                                   Tempo::Beat beat) {
  ControlMessage *retVal = newMessage(sizeof(AddNoteRequest), ADD_NOTE_REQUEST);
  if (retVal) {
    AddNoteRequest *request = (AddNoteRequest *)(retVal + 1);
    request->note = tone;
    request->octave = octave;
    request->duration = beat;
  }
  return retVal;
}
ControlMessage *makeAddNoteResponse(Semitone tone, int octave,
                                    Tempo::Beat beat) {
  ControlMessage *retVal =
      newMessage(sizeof(AddNoteResponse), ADD_NOTE_RESPONSE);
  if (retVal) {
    AddNoteResponse *response = (AddNoteResponse *)(retVal + 1);
    response->note = tone;
    response->octave = octave;
    response->duration = beat;
  }
  return retVal;
}
ControlMessage *makeAddRestRequest(Tempo::Beat beat) {
  ControlMessage *retVal = newMessage(sizeof(AddRestRequest), ADD_REST_REQUEST);
  if (retVal) {
    AddRestRequest *request = (AddRestRequest *)(retVal + 1);
    request->duration = beat;
  }
  return retVal;
}
ControlMessage *makeAddRestResponse(Tempo::Beat beat) {
  ControlMessage *retVal =
      newMessage(sizeof(AddRestResponse), ADD_REST_RESPONSE);
  if (retVal) {
    AddRestResponse *response = (AddRestResponse *)(retVal + 1);
    response->duration = beat;
  }
  return retVal;
}
ControlMessage *makeSetTrackGainRequest(int64_t gain) {
  ControlMessage *retVal =
      newMessage(sizeof(SetTrackGainRequest), SET_TRACK_GAIN_REQUEST);
  if (retVal) {
    SetTrackGainRequest *request = (SetTrackGainRequest *)(retVal + 1);
    request->gain = gain;
  }
  return retVal;
}
ControlMessage *makeSetTrackGainResponse(int64_t gain) {
  ControlMessage *retVal =
      newMessage(sizeof(SetTrackGainResponse), SET_TRACK_GAIN_RESPONSE);
  if (retVal) {
    SetTrackGainResponse *response = (SetTrackGainResponse *)(retVal + 1);
    response->gain = gain;
  }
  return retVal;
}
ControlMessage *makeAddDelayRequest(uint32_t mix, uint32_t delay,
                                    Tempo::Beat beatDelay, uint32_t feedback,
                                    uint64_t priority) {
  ControlMessage *retVal =
      newMessage(sizeof(AddDelayRequest), ADD_DELAY_REQUEST);
  if (retVal) {
    AddDelayRequest *adr = (AddDelayRequest *)(retVal + 1);
    adr->mix = mix;
    adr->delay = delay;
    adr->beatDelay = beatDelay;
    adr->feedback = feedback;
    adr->priority = priority;
  }
  return retVal;
}
ControlMessage *makeAddDelayResponse(uint32_t mix, uint32_t delay,
                                     Tempo::Beat beatDelay, uint32_t feedback,
                                     uint64_t priority) {
  ControlMessage *retVal =
      newMessage(sizeof(AddDelayResponse), ADD_DELAY_RESPONSE);
  if (retVal) {
    AddDelayResponse *adr = (AddDelayResponse *)(retVal + 1);
    adr->mix = mix;
    adr->delay = delay;
    adr->beatDelay = beatDelay;
    adr->feedback = feedback;
    adr->priority = priority;
  }
  return retVal;
}
ControlMessage *makeAddChorusRequest(uint32_t mix, uint32_t delay,
                                     uint32_t depth, uint32_t rate,
                                     uint32_t priority) {
  ControlMessage *retVal =
      newMessage(sizeof(AddChorusRequest), ADD_CHORUS_REQUEST);
  if (retVal) {
    AddChorusRequest *acr = (AddChorusRequest *)(retVal + 1);
    acr->mix = mix;
    acr->delay = delay;
    acr->depth = depth;
    acr->rate = rate;
    acr->priority = priority;
  }
  return retVal;
}
ControlMessage *makeAddChorusResponse(uint32_t mix, uint32_t delay,
                                      uint32_t depth, uint32_t rate,
                                      uint32_t priority) {
  ControlMessage *retVal =
      newMessage(sizeof(AddChorusResponse), ADD_CHORUS_RESPONSE);
  if (retVal) {
    AddChorusResponse *acr = (AddChorusResponse *)(retVal + 1);
    acr->mix = mix;
    acr->delay = delay;
    acr->depth = depth;
    acr->rate = rate;
    acr->priority = priority;
  }
  return retVal;
}
ControlMessage *makeAddTremoloRequest(uint32_t mix, uint32_t rate,
                                      uint32_t mode, uint64_t priority) {
  ControlMessage *retVal =
      newMessage(sizeof(AddTremoloRequest), ADD_TREMOLO_REQUEST);
  if (retVal) {
    AddTremoloRequest *atr = (AddTremoloRequest *)(retVal + 1);
    atr->mix = mix;
    atr->rate = rate;
    atr->priority = priority;
  }
  return retVal;
}
ControlMessage *makeAddTremoloResponse(uint32_t mix, uint32_t rate,
                                       uint32_t mode, uint64_t priority) {
  ControlMessage *retVal =
      newMessage(sizeof(AddTremoloResponse), ADD_TREMOLO_RESPONSE);
  if (retVal) {
    AddTremoloResponse *atr = (AddTremoloResponse *)(retVal + 1);
    atr->mix = mix;
    atr->rate = rate;
    atr->priority = priority;
  }
  return retVal;
}
ControlMessage *makeAddEqBandRequest(uint16_t frequency, int16_t gain,
                                     uint32_t bandwidth, uint64_t priority) {
  ControlMessage *retVal =
      newMessage(sizeof(AddEqBandRequest), ADD_EQ_BAND_REQUEST);
  if (retVal) {
    AddEqBandRequest *aer = (AddEqBandRequest *)(retVal + 1);
    aer->frequency = frequency;
    aer->gain = gain;
    aer->bandwidth = bandwidth;
    aer->priority = priority;
  }
  return retVal;
}
ControlMessage *makeAddEqBandResponse(uint16_t frequency, int16_t gain,
                                      uint32_t bandwidth, uint64_t priority) {
  ControlMessage *retVal =
      newMessage(sizeof(AddEqBandResponse), ADD_EQ_BAND_RESPONSE);
  if (retVal) {
    AddEqBandResponse *aer = (AddEqBandResponse *)(retVal + 1);
    aer->frequency = frequency;
    aer->gain = gain;
    aer->bandwidth = bandwidth;
    aer->priority = priority;
  }
  return retVal;
}
ControlMessage *makeRemoveEffectRequest(uint32_t priority) {
  ControlMessage *retVal =
      newMessage(sizeof(RemoveEffectRequest), REMOVE_EFFECT_REQUEST);
  if (retVal) {
    RemoveEffectRequest *request = (RemoveEffectRequest *)(retVal + 1);
    request->priority = priority;
  }
  return retVal;
}
ControlMessage *makeRemoveEffectResponse(uint32_t priority) {
  ControlMessage *retVal =
      newMessage(sizeof(RemoveEffectResponse), REMOVE_EFFECT_RESPONSE);
  if (retVal) {
    RemoveEffectResponse *response = (RemoveEffectResponse *)(retVal + 1);
    response->priority = priority;
  }
  return retVal;
}
ControlMessage *makeCreateSongRequest(uint64_t tempo) {
  ControlMessage *retVal =
      newMessage(sizeof(CreateSongRequest), CREATE_SONG_REQUEST);
  if (retVal) {
    CreateSongRequest *request = (CreateSongRequest *)(retVal + 1);
    request->tempo = tempo;
  }
  return retVal;
}
ControlMessage *makeCreateSongResponse(uint64_t tempo) {
  ControlMessage *retVal =
      newMessage(sizeof(CreateSongResponse), CREATE_SONG_RESPONSE);
  if (retVal) {
    CreateSongResponse *response = (CreateSongResponse *)(retVal + 1);
    response->tempo = tempo;
  }
  return retVal;
}
ControlMessage *makeBounceSongRequest(uint64_t clearSong) {
  ControlMessage *retVal =
      newMessage(sizeof(BounceSongRequest), BOUNCE_SONG_REQUEST);
  if (retVal) {
    BounceSongRequest *request = (BounceSongRequest *)(retVal + 1);
    request->clearSong = clearSong;
  }
  return retVal;
}
ControlMessage *makeBounceSongResponse(uint64_t size, uint8_t *data) {
  ControlMessage *retVal =
      newMessage(sizeof(BounceSongResponse) + size - 1, BOUNCE_SONG_RESPONSE);
  if (retVal) {
    BounceSongResponse *response = (BounceSongResponse *)(retVal + 1);
    response->size = size;
    memcpy(response->data, data, size);
  }
  return retVal;
}
ControlMessage *makeSaveStateRequest() {
  ControlMessage *retVal = newMessage(0, SAVE_STATE_REQUEST);
  return retVal;
}
ControlMessage *makeSaveStateResponse(uint64_t size, uint8_t *data) {
  ControlMessage *retVal =
      newMessage(sizeof(SaveStateResponse) + size - 1, SAVE_STATE_RESPONSE);
  if (retVal) {
    SaveStateResponse *response = (SaveStateResponse *)(retVal + 1);
    response->size = size;
    memcpy(response->data, data, size);
  }
  return retVal;
}
ControlMessage *makeShutdownRequest() {
  ControlMessage *retVal = newMessage(0, SHUTDOWN_REQUEST);
  return retVal;
}
ControlMessage *makeShutdownResponse() {
  ControlMessage *retVal = newMessage(0, SHUTDOWN_RESPONSE);
  return retVal;
}
ControlMessage *makeSpecialKeyRequest() {
  ControlMessage *retVal = newMessage(0, SPECIAL_KEY_REQUEST);
  return retVal;
}
ControlMessage *makeSpecialKeyResponse(const char *key) {
  ControlMessage *retVal =
      newMessage(sizeof(SpecialKeyResponse), SPECIAL_KEY_RESPONSE);
  if (retVal) {
    SpecialKeyResponse *response = (SpecialKeyResponse *)(retVal + 1);
    ::snprintf((char *)response->key, sizeof(response->key) - 1, "%s", key);
  }
  return retVal;
}
ControlMessage *makeErrorResponse(uint64_t errorCode) {
  ControlMessage *retVal = newMessage(sizeof(ErrorResponse), ERROR_RESPONSE);
  if (retVal) {
    ErrorResponse *response = (ErrorResponse *)(retVal + 1);
    response->errorCode = errorCode;
  }
  return retVal;
}
ControlMessage *makeSongInfoRequest() {
  ControlMessage *retVal = newMessage(0, SONG_INFO_REQUEST);
  return retVal;
}
ControlMessage *makeSongInfoResponse(uint32_t numTracks, uint32_t lenSamples,
                                     uint64_t lenMillis, const char *name) {
  ControlMessage *retVal =
      newMessage(sizeof(SongInfoResponse), SONG_INFO_RESPONSE);
  if (retVal) {
    SongInfoResponse *sir = (SongInfoResponse *)(retVal + 1);
    sir->numTracks = numTracks;
    sir->lenSamples = lenSamples;
    sir->lenMillis = lenMillis;
    ::snprintf((char *)sir->name, sizeof(sir->name) - 1, "%s", name);
  }
  return retVal;
}
ControlMessage *makeClearSongRequest() {
  ControlMessage *retVal = newMessage(0, CLEAR_SONG_REQUEST);
  return retVal;
}
ControlMessage *makeClearSongResponse() {
  ControlMessage *retVal = newMessage(0, CLEAR_SONG_RESPONSE);
  return retVal;
}
ControlMessage *makeSetNameRequest(const char *name) {
  ControlMessage *retVal =
      newMessage(sizeof(SetSongNameRequest), SET_SONG_NAME_REQUEST);
  if (retVal) {
    SetSongNameRequest *ssnr = (SetSongNameRequest *)(retVal + 1);
    ::snprintf((char *)ssnr->name, sizeof(ssnr->name) - 1, "%s", name);
  }
  return retVal;
}
ControlMessage *makeSetNameResponse(const char *name) {
  ControlMessage *retVal =
      newMessage(sizeof(SetSongNameResponse), SET_SONG_NAME_RESPONSE);
  if (retVal) {
    SetSongNameResponse *ssnr = (SetSongNameResponse *)(retVal + 1);
    ::snprintf((char *)ssnr->name, sizeof(ssnr->name) - 1, "%s", name);
  }
  return retVal;
}
ControlMessage *parseMessage(uint8_t *message, int bufferSize) {
  ControlMessage *retVal = NULL;
  if (bufferSize >= sizeof(ControlMessage)) {
    ControlMessage *base = (ControlMessage *)message;
    if (bufferSize >= base->messageSize) {
      retVal = (ControlMessage *)malloc(base->messageSize);
      if (retVal) {
        memcpy(retVal, message, base->messageSize);
      }
    }
  }
  return retVal;
}