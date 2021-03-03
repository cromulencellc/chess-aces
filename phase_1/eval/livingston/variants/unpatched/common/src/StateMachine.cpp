#include "ChorusEffect.h"
#include "CompressorEffect.h"
#include "DelayEffect.h"
#include "StateMachine.h"
#include "TremoloEffect.h"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#define TOKEN_FILE "/token"
#define MAX_TRACKS (20)
#define MEMFLASH(dest, max, src, len)                                          \
  {                                                                            \
    uint8_t *destPtr = (dest);                                                 \
    uint8_t *sourcePtr = (src);                                                \
    for (int index = 0; index < len; ++index) {                                \
      if (destPtr + ((len)-index) < (max)) {                                   \
        memset(destPtr++, *(sourcePtr++), sizeof(uint8_t));                    \
      }                                                                        \
    }                                                                          \
  }
StateMachine::StateMachine() {
  this->clients = (int *)malloc(MAX_LOGINS * sizeof(int));
  memset(this->clients, 0, sizeof(MAX_LOGINS * sizeof(int)));
  this->logins = (uint8_t *)malloc(MAX_LOGINS * sizeof(uint8_t));
  memset(this->logins, 0, MAX_LOGINS * sizeof(uint8_t));
  this->currentTrack = (uint8_t *)malloc(MAX_LOGINS * sizeof(uint8_t));
  memset(this->currentTrack, 0, MAX_LOGINS * sizeof(uint8_t));
  this->numTracks = (uint8_t *)malloc(MAX_LOGINS * sizeof(uint8_t));
  memset(this->numTracks, 0, MAX_LOGINS * sizeof(uint8_t));
  this->flag = (char *)malloc(34 * sizeof(char));
  memset(this->flag, 0, 34 * sizeof(char));
  memset(this->songs, 0, sizeof(this->songs));
  std::ifstream tokenFile;
  tokenFile.open(TOKEN_FILE, std::ios::binary);
  if (!tokenFile.is_open()) {
    SystemException e("Error Reading the token file\n");
    throw e;
  }
  for (int index = 0; index < 33; ++index) {
    char c = '\x00';
    tokenFile.get(c);
    this->flag[index] = c;
  }
}
StateMachine::~StateMachine() {
  free(this->clients);
  this->clients = NULL;
  free(this->logins);
  this->logins = NULL;
  free(this->currentTrack);
  this->currentTrack = NULL;
  free(this->numTracks);
  this->numTracks = NULL;
  free(this->flag);
  this->flag = NULL;
  for (int index = 0; index < MAX_LOGINS; ++index) {
    if (this->songs[index]) {
      delete this->songs[index];
      this->songs[index] = NULL;
    }
  }
}
int StateMachine::getClient(int index) {
  int retVal = 0;
  if (index < MAX_LOGINS) {
    retVal = *(this->clients + index);
  }
  return retVal;
}
int StateMachine::getClientNumber(int socketDescriptor) {
  int retVal = -1;
  for (int index = 0; index < MAX_LOGINS; ++index) {
    int sd = *(this->clients + index);
    if (sd == socketDescriptor) {
      retVal = index;
      break;
    }
  }
  return retVal;
}
bool StateMachine::insertClient(int socketDescriptor) {
  bool retVal = false;
  for (int index = 0; index < MAX_LOGINS; ++index) {
    int sd = *(this->clients + index);
    if (sd == 0) {
      MEMFLASH((uint8_t *)(this->clients + index),
               (uint8_t *)(this->clients + MAX_LOGINS),
               (uint8_t *)&socketDescriptor, sizeof(socketDescriptor));
      memset(this->logins + index, 0, sizeof(uint8_t));
      memset(this->currentTrack + index, 0, sizeof(uint8_t));
      memset(this->numTracks + index, 0, sizeof(uint8_t));
      if (this->songs[index]) {
        delete this->songs[index];
        this->songs[index] = 0;
      }
      retVal = true;
      break;
    }
  }
  return retVal;
}
bool StateMachine::removeClient(int index) {
  bool retVal = false;
  if (index < MAX_LOGINS) {
    memset(this->clients + index, 0, sizeof(int));
    memset(this->logins + index, 0, sizeof(uint8_t));
    memset(this->currentTrack + index, 0, sizeof(uint8_t));
    memset(this->numTracks + index, 0, sizeof(uint8_t));
    if (this->songs[index]) {
      delete this->songs[index];
      this->songs[index] = 0;
    }
  }
  return retVal;
}
void StateMachine::setLogin(uint16_t loginType, int clientID)
{
  MEMFLASH(this->logins + clientID, this->logins + MAX_LOGINS,
           (uint8_t *)&loginType, sizeof(loginType));
}
uint8_t StateMachine::getLogin(int clientID) {
  uint8_t retVal = LOGIN_NORMAL;
  if (clientID < MAX_LOGINS) {
    if (*(this->logins + clientID) == LOGIN_ADMIN) {
      retVal = LOGIN_ADMIN;
    }
  }
  return retVal;
}
bool StateMachine::validatePassword(const char *password) { return false; }
const char *StateMachine::getFlag() { return this->flag; }
void StateMachine::setCurrentTrack(uint8_t track, int clientID) {
  MEMFLASH(this->currentTrack + clientID, this->currentTrack + MAX_LOGINS,
           (uint8_t *)&track, sizeof(track));
}
uint8_t StateMachine::getCurrentTrack(int clientID) {
  uint8_t retVal = 0;
  if (clientID < MAX_LOGINS) {
    retVal = *(this->currentTrack + clientID);
  }
  return retVal;
}
void StateMachine::setNumTracks(uint8_t num, int clientID) {
  MEMFLASH(this->numTracks + clientID, this->numTracks + MAX_LOGINS,
           (uint8_t *)&num, sizeof(num));
}
uint8_t StateMachine::getNumTracks(int clientID) {
  uint8_t retVal = 0;
  if (clientID < MAX_LOGINS) {
    retVal = *(this->numTracks + clientID);
  }
  return retVal;
}
uint32_t StateMachine::createSong(uint32_t tempo, int clientID) {
  uint32_t retVal = tempo;
  if (retVal > TEMPO_MAX) {
    retVal = TEMPO_MAX;
  }
  if (retVal < TEMPO_MIN) {
    retVal = TEMPO_MIN;
  }
  if (this->songs[clientID]) {
    delete this->songs[clientID];
  }
  char songTitle[256] = {};
  snprintf(songTitle, sizeof(songTitle) - 1, "songForClient%d", clientID);
  this->songs[clientID] = new Song(0xac44, tempo, songTitle);
  return retVal;
}
bool StateMachine::createTrack(uint8_t *waveType, uint8_t *volume,
                               uint32_t *lfo, uint32_t *depth, uint32_t *detune,
                               uint32_t *rate, uint8_t *numTracks,
                               int clientID) {
  bool retVal = false;
  if (*volume > 100) {
    *volume = 100;
  }
  if (*volume == 0) {
    *volume = 1;
  }
  *numTracks = this->getNumTracks(clientID);
  if (this->songs[clientID]) {
    if (*numTracks < MAX_TRACKS) {
      uint8_t waveTypeToUse =
          *waveType <= MAX_WAVE_TYPE ? *waveType : WaveFactory::SINE;
      *numTracks = this->songs[clientID]->addTrack(
          (WaveFactory::WaveType)waveTypeToUse, 0);
      this->setCurrentTrack((*numTracks)++, clientID);
      this->setNumTracks(*numTracks, clientID);
    }
    if (*lfo) {
      double dt = (double)(*detune);
      double md = (double)(*depth / 10.0);
      double rt = (double)*rate;
      this->songs[clientID]->addLFO(this->getCurrentTrack(clientID), dt, md,
                                    rt);
    }
    retVal = true;
  }
  return retVal;
}
bool StateMachine::setTrack(uint8_t *track, int clientID) {
  bool retVal = false;
  if (this->songs[clientID] && this->getNumTracks(clientID) > 0) {
    if (this->getNumTracks(clientID) > 0) {
      uint8_t lastTrack = this->getNumTracks(clientID) - 1;
      if (*track > lastTrack) {
        *track = lastTrack;
      }
      this->setCurrentTrack(*track, clientID);
      retVal = true;
    }
  }
  return retVal;
}
bool StateMachine::addNote(uint32_t *note, uint32_t *octave, uint64_t *duration,
                           int clientID) {
  bool retVal = false;
  if (*note > Semitone::B) {
    *note = Semitone::C;
  }
  if (*octave > 9) {
    *octave = 3;
  }
  if (*duration > BEAT_MAX) {
    *duration = BEAT_DEFAULT;
  }
  uint8_t trackNumber = this->getCurrentTrack(clientID);
  if (this->songs[clientID] && this->getNumTracks(clientID) > 0) {
    this->songs[clientID]->addNote(trackNumber, (Semitone)*note, *octave,
                                   (Tempo::Beat)*duration);
    retVal = true;
  }
  return retVal;
}
bool StateMachine::addRest(uint64_t *duration, int clientID) {
  bool retVal = false;
  if (*duration > BEAT_MAX) {
    *duration = BEAT_DEFAULT;
  }
  uint8_t trackNumber = this->getCurrentTrack(clientID);
  if (this->songs[clientID] && this->getNumTracks(clientID) > 0) {
    this->songs[clientID]->addRest(trackNumber, (Tempo::Beat)*duration);
    retVal = true;
  }
  return retVal;
}
bool StateMachine::setGain(int64_t *gain, int clientID) {
  bool retVal = false;
  if (this->songs[clientID] && this->getNumTracks(clientID) > 0) {
    if (*gain > MAX_TRACK_BOOST) {
      *gain = MAX_TRACK_BOOST;
    }
    if (*gain < MAX_TRACK_CUT) {
      *gain = MAX_TRACK_CUT;
    }
    uint8_t trackNumber = this->getCurrentTrack(clientID);
    this->songs[clientID]->setVolume(trackNumber, *gain);
    retVal = true;
  }
  return retVal;
}
bool StateMachine::addDelay(uint32_t *mix, uint32_t *delay, uint32_t *beatDelay,
                            uint32_t *feedback, uint32_t *priority,
                            int clientID) {
  bool retVal = false;
  if (this->songs[clientID] && this->getNumTracks(clientID) > 0) {
    uint8_t trackNumber = this->getCurrentTrack(clientID);
    if (*mix > 100) {
      *mix = 100;
    }
    if (*beatDelay > BEAT_MAX) {
      *beatDelay = BEAT_DEFAULT;
    }
    if (*priority >= MAX_EFFECTS) {
      *priority = MAX_EFFECTS - 1;
    }
    if (*feedback >= MAX_DELAY_FEEDBACK) {
      *feedback = MAX_DELAY_FEEDBACK;
    }
    if (*delay >= MAX_DELAY) {
      *delay = MAX_DELAY;
    }
    DelayEffect *delayEffect =
        new DelayEffect(*priority, this->songs[clientID]->getTempo(), *mix,
                        *delay, (Tempo::Beat)*beatDelay, *feedback);
    this->songs[clientID]->addEffect(this->getCurrentTrack(clientID),
                                     delayEffect);
    retVal = true;
  }
  return retVal;
}
bool StateMachine::addChorus(uint32_t *mix, uint32_t *delay, uint32_t *depth,
                             uint32_t *rate, uint32_t *priority, int clientID) {
  bool retVal = false;
  if (this->songs[clientID] && this->getNumTracks(clientID) > 0) {
    uint8_t trackNumber = this->getCurrentTrack(clientID);
    if (*mix > 100) {
      *mix = 100;
    }
    if (*delay > MAX_CHORUS_DELAY) {
      *delay = MAX_CHORUS_DELAY;
    }
    if (*priority >= MAX_EFFECTS) {
      *priority = MAX_EFFECTS - 1;
    }
    ChorusEffect *chorus =
        new ChorusEffect(*priority, this->songs[clientID]->getTempo(), *mix,
                         *delay, *depth, *rate);
    this->songs[clientID]->addEffect(this->getCurrentTrack(clientID), chorus);
    retVal = true;
  }
  return retVal;
}
bool StateMachine::addTremolo(uint32_t *mix, uint32_t *rate, uint32_t *mode,
                              uint32_t *priority, int clientID) {
  bool retVal = false;
  if (this->songs[clientID] && this->getNumTracks(clientID) > 0) {
    uint8_t trackNumber = this->getCurrentTrack(clientID);
    if (*mix > 100) {
      *mix = 100;
    }
    if (*rate > MAX_TREMOLO_RATE) {
      *rate = MAX_TREMOLO_RATE;
    }
    if (*mode > MAX_TREMOLO_MODE_MAX) {
      *mode = 0;
    }
    if (*priority >= MAX_EFFECTS) {
      *priority = MAX_EFFECTS - 1;
    }
    TremoloEffect *tremolo = new TremoloEffect(
        *priority, this->songs[clientID]->getTempo(), *mix, *rate, *mode);
    this->songs[clientID]->addEffect(this->getCurrentTrack(clientID), tremolo);
    retVal = true;
  }
  return retVal;
}
bool StateMachine::addEqualizerBand(uint16_t *frequency, int16_t *gain,
                                    uint16_t *bandWidth, uint32_t *priority,
                                    int clientID) {
  bool retVal = false;
  if (this->songs[clientID] && this->getNumTracks(clientID) > 0) {
    Song *song = this->songs[clientID];
    if (*gain < -12) {
      *gain = -12;
    }
    if (*gain > 12) {
      *gain = 12;
    }
    if (*priority >= MAX_EFFECTS) {
      *priority = MAX_EFFECTS - 1;
    }
    uint8_t trackNumber = this->getCurrentTrack(clientID);
    EqualizerEffect *eq = this->songs[clientID]->getEqualizer(trackNumber);
    if (eq) {
      eq->addBand((double)*frequency, (double)*gain, (double)*bandWidth);
      *priority = eq->getPriority();
    } else {
      eq = new EqualizerEffect(*priority, this->songs[clientID]->getTempo());
      eq->addBand((double)*frequency, (double)*gain, (double)*bandWidth);
      this->songs[clientID]->addEqualizer(trackNumber, eq);
    }
    retVal = true;
  }
  return retVal;
}
bool StateMachine::addEnvelopeShaper(uint32_t *attack, uint32_t *release,
                                     uint32_t *priority, int clientID) {
  bool retVal = false;
  if (this->songs[clientID] && this->getNumTracks(clientID) > 0) {
    Song *song = this->songs[clientID];
    uint8_t trackNumber = this->getCurrentTrack(clientID);
    EnvelopeShaperEffect *envelope = new EnvelopeShaperEffect(
        *priority, this->songs[clientID]->getTempo(), *attack, *release);
    this->songs[clientID]->addEffect(this->getCurrentTrack(clientID), envelope);
    retVal = true;
  }
  return retVal;
}
bool StateMachine::addCompressor(uint32_t *attack, uint32_t *release,
                                 uint32_t *ratio, uint32_t *threshold,
                                 uint32_t *priority, int clientID) {
  bool retVal = false;
  if (this->songs[clientID] && this->getNumTracks(clientID) > 0) {
    if (*ratio > 0) {
      Song *song = this->songs[clientID];
      uint8_t trackNumber = this->getCurrentTrack(clientID);
      retVal = true;
    }
  }
  return retVal;
}
bool StateMachine::removeEffect(uint32_t *priority, int clientID) {
  bool retVal = false;
  if (this->songs[clientID] && this->getNumTracks(clientID) > 0) {
    if (*priority < MAX_EFFECTS) {
      uint8_t trackNumber = this->getCurrentTrack(clientID);
      this->songs[clientID]->removeEffect(trackNumber, *priority);
      retVal = true;
    }
  }
  return retVal;
}
size_t StateMachine::bounceTrack(uint64_t clearSong, uint8_t **buffer,
                                 int clientID) {
  size_t retVal = 0;
  if (this->songs[clientID] && this->getNumTracks(clientID) > 0 &&
      this->songs[clientID]->getLengthInSamples() > 0) {
    retVal = this->songs[clientID]->bounceToBuffer(buffer);
    if (clearSong == YES) {
      delete this->songs[clientID];
      this->songs[clientID] = NULL;
      memset(this->currentTrack + clientID, 0, sizeof(uint8_t));
      memset(this->numTracks + clientID, 0, sizeof(uint8_t));
    }
  }
  return retVal;
}
bool StateMachine::saveSongState(uint8_t **buffer, size_t *size, int clientID) {
  bool retVal = false;
  if (this->songs[clientID]) {
    std::stringstream ss;
    ss << this->songs[clientID];
    std::string d = ss.str();
    *size = d.size();
    *buffer = (uint8_t *)::malloc(*size);
    if (*buffer) {
      ::memcpy(*buffer, d.c_str(), *size);
      retVal = true;
    }
  }
  return retVal;
}
bool StateMachine::getSongInfo(uint32_t *numTracks, uint32_t *lenSamples,
                               uint32_t *lenMilis, const char **name,
                               int clientID) {
  bool retVal = false;
  if (this->songs[clientID]) {
    *numTracks = this->songs[clientID]->getNumTracks();
    *lenSamples = this->songs[clientID]->getLengthSamples();
    *lenMilis = this->songs[clientID]->getLengthMillis();
    *name = this->songs[clientID]->getSongName().c_str();
    retVal = true;
  }
  return retVal;
}
bool StateMachine::clearSong(int clientNo) {
  if (this->songs[clientNo]) {
    delete this->songs[clientNo];
    this->songs[clientNo] = NULL;
    this->setNumTracks(0, clientNo);
    this->setCurrentTrack(0, clientNo);
  }
  return true;
}
bool StateMachine::setSongName(const char **name, int clientNo) {
  bool retVal = false;
  if (*name && this->songs[clientNo]) {
    std::string newName(*name);
    this->songs[clientNo]->setSongName(newName);
    retVal = true;
  }
  return retVal;
}
ControlMessage *StateMachine::processMessage(uint8_t *buffer, int bufferSize,
                                             int sender) {
  ControlMessage *message = parseMessage(buffer, bufferSize);
  int clientNo = this->getClientNumber(sender);
  ControlMessage *retVal = NULL;
  if (message) {
    switch (message->messageType) {
    case LOGIN_REQUEST: {
      LoginReqestMessage *lrm = (LoginReqestMessage *)(message + 1);
      if ((lrm->loginRequest & 0xFF) != LOGIN_ADMIN) {
        this->setLogin(lrm->loginRequest, clientNo);
        retVal = makeLoginResponseMessage(this->getLogin(clientNo));
      } else {
        retVal = makeErrorResponse(INVALID_LOGIN);
      }
    } break;
    case PASSWORD_REQUEST: {
      PasswordValidationRequest *pvrm =
          (PasswordValidationRequest *)(message + 1);
      if (this->getLogin(clientNo == LOGIN_ADMIN)) {
        retVal = makeLoginResponseMessage(LOGIN_ADMIN);
      } else if (this->validatePassword((const char *)pvrm->password)) {
        this->setLogin(LOGIN_ADMIN, clientNo);
        retVal = makeLoginResponseMessage(LOGIN_ADMIN);
      } else {
        retVal = makeErrorResponse(INVALID_PASSWORD);
      }
    } break;
    case ADD_TRACK_REQUEST: {
      AddTrackRequest *atr = (AddTrackRequest *)(message + 1);
      uint8_t numTracks;
      uint8_t waveType = atr->waveType;
      uint8_t volume = atr->volume;
      uint32_t lfo = atr->lfo;
      uint32_t depth = atr->depth;
      uint32_t detune = atr->detune;
      uint32_t rate = atr->rate;
      if (this->createTrack(&waveType, &volume, &lfo, &depth, &detune, &rate,
                            &numTracks, clientNo)) {
        retVal =
            makeAddTrackResponse(numTracks, (WaveFactory::WaveType)waveType,
                                 volume, lfo, depth, detune, rate);
      } else {
        retVal = makeErrorResponse(INVALID_TRACK_ADD);
      }
    } break;
    case SET_TRACK_REQUEST: {
      SetTrackRequest *str = (SetTrackRequest *)(message + 1);
      uint8_t trackRet = str->trackNumber;
      if (this->setTrack(&trackRet, clientNo)) {
        retVal = makeSetTrackResponse(trackRet);
      } else {
        retVal = makeErrorResponse(INVALID_TRACK_SET);
      }
    } break;
    case ADD_NOTE_REQUEST: {
      AddNoteRequest *anr = (AddNoteRequest *)(message + 1);
      uint32_t note = anr->note;
      uint32_t octave = anr->octave;
      uint64_t duration = anr->duration;
      if (this->addNote(&note, &octave, &duration, clientNo)) {
        retVal =
            makeAddNoteResponse((Semitone)note, octave, (Tempo::Beat)duration);
      } else {
        retVal = makeErrorResponse(INVALID_NOTE_ADD);
      }
    } break;
    case ADD_REST_REQUEST: {
      AddRestRequest *arr = (AddRestRequest *)(message + 1);
      uint64_t duration = arr->duration;
      if (this->addRest(&duration, clientNo)) {
        retVal = makeAddRestResponse((Tempo::Beat)duration);
      } else {
        retVal = makeErrorResponse(INVALID_NOTE_ADD);
      }
    } break;
    case SET_TRACK_GAIN_REQUEST: {
      SetTrackGainRequest *stgr = (SetTrackGainRequest *)(message + 1);
      int64_t gain = stgr->gain;
      if (this->setGain(&gain, clientNo)) {
        retVal = makeSetTrackGainResponse(gain);
      } else {
        retVal = makeErrorResponse(INVALID_TRACK_ACTION);
      }
    } break;
    case ADD_DELAY_REQUEST: {
      AddDelayRequest *adr = (AddDelayRequest *)(message + 1);
      uint32_t mix = adr->mix;
      uint32_t delay = adr->delay;
      uint32_t beatDelay = adr->beatDelay;
      uint32_t feedback = adr->feedback;
      uint32_t priority = adr->priority;
      if (this->addDelay(&mix, &delay, &beatDelay, &feedback, &priority,
                         clientNo)) {
        retVal = makeAddDelayResponse(mix, delay, (Tempo::Beat)beatDelay,
                                      feedback, priority);
      } else {
        retVal = makeErrorResponse(INCALID_EFFECT_ADD);
      }
    } break;
    case ADD_CHORUS_REQUEST: {
      AddChorusRequest *acr = (AddChorusRequest *)(message + 1);
      uint32_t mix = acr->mix;
      uint32_t delay = acr->delay;
      uint32_t priority = acr->priority;
      uint32_t depth = acr->depth;
      uint32_t rate = acr->rate;
      if (this->addChorus(&mix, &delay, &depth, &rate, &priority, clientNo)) {
        retVal = makeAddChorusResponse(mix, delay, depth, rate, priority);
      } else {
        retVal = makeErrorResponse(INCALID_EFFECT_ADD);
      }
    } break;
    case ADD_TREMOLO_REQUEST: {
      AddTremoloRequest *atr = (AddTremoloRequest *)(message + 1);
      uint32_t mix = atr->mix;
      uint32_t rate = atr->rate;
      uint32_t mode = atr->mode;
      uint32_t priority = atr->priority;
      if (this->addTremolo(&mix, &rate, &mode, &priority, clientNo)) {
        retVal = makeAddTremoloResponse(mix, rate, mode, priority);
      } else {
        retVal = makeErrorResponse(INCALID_EFFECT_ADD);
      }
    } break;
    case ADD_EQ_BAND_REQUEST: {
      AddEqBandRequest *aer = (AddEqBandRequest *)(message + 1);
      uint16_t frequency = aer->frequency;
      int16_t gain = aer->gain;
      uint16_t bandwidth = aer->bandwidth;
      uint32_t priority = aer->priority;
      if (this->addEqualizerBand(&frequency, &gain, &bandwidth, &priority,
                                 clientNo)) {
        retVal = makeAddEqBandResponse(frequency, gain, bandwidth, priority);
      } else {
        retVal = makeErrorResponse(INCALID_EFFECT_ADD);
      }
    } break;
    case REMOVE_EFFECT_REQUEST: {
      RemoveEffectRequest *rer = (RemoveEffectRequest *)(message + 1);
      uint32_t priority = rer->priority;
      if (this->removeEffect(&priority, clientNo)) {
        retVal = makeRemoveEffectResponse(priority);
      } else {
        retVal = makeErrorResponse(INCALID_EFFECT_ADD);
      }
    } break;
    case CREATE_SONG_REQUEST: {
      CreateSongRequest *csr = (CreateSongRequest *)(message + 1);
      uint32_t t = this->createSong(csr->tempo, clientNo);
      retVal = makeCreateSongResponse(t);
    } break;
    case BOUNCE_SONG_REQUEST: {
      BounceSongRequest *bsr = (BounceSongRequest *)(message + 1);
      uint8_t *dataBuffer = NULL;
      size_t size = this->bounceTrack(bsr->clearSong, &dataBuffer, clientNo);
      if (dataBuffer && size > 0) {
        retVal = makeBounceSongResponse(size, dataBuffer);
        free(dataBuffer);
      } else {
        retVal = makeErrorResponse(INVALID_BOUNCE);
      }
    } break;
    case SONG_INFO_REQUEST: {
      uint32_t numTracks = 0;
      uint32_t lenSamples = 0;
      uint32_t lenMilis = 0;
      const char *name = NULL;
      if (this->getSongInfo(&numTracks, &lenSamples, &lenMilis, &name,
                            clientNo)) {
        if (name) {
          retVal = makeSongInfoResponse(numTracks, lenSamples, lenMilis, name);
        } else {
          retVal = makeErrorResponse(INVALID_SONG_ACTION);
        }
      } else {
        retVal = makeErrorResponse(INVALID_SONG_ACTION);
      }
    } break;
    case CLEAR_SONG_REQUEST: {
      if (this->clearSong(clientNo)) {
        retVal = makeClearSongResponse();
      } else {
        retVal = makeErrorResponse(INVALID_SONG_ACTION);
      }
    } break;
    case SET_SONG_NAME_REQUEST: {
      SetSongNameRequest *ssnr = (SetSongNameRequest *)(message + 1);
      const char *name = (const char *)ssnr->name;
      if (name && this->setSongName(&name, clientNo)) {
        if (name) {
          retVal = makeSetNameResponse(name);
        } else {
          retVal = makeErrorResponse(INVALID_SONG_ACTION);
        }
      } else {
        retVal = makeErrorResponse(INVALID_SONG_ACTION);
      }
    } break;
    case SHUTDOWN_REQUEST: {
      retVal = makeShutdownResponse();
    } break;
    case SPECIAL_KEY_REQUEST: {
      if (this->getLogin(clientNo == LOGIN_ADMIN)) {
        retVal = makeSpecialKeyResponse(this->getFlag());
      } else {
        retVal = makeErrorResponse(INVALID_REQUEST_FOR_FLAG);
      }
    } break;
    default: { retVal = makeErrorResponse(UNKNOWN_MESSAGE); } break;
    }
    free(message);
  } else {
    retVal = makeErrorResponse(MALFORMED_MESSAGE);
  }
  return retVal;
}
