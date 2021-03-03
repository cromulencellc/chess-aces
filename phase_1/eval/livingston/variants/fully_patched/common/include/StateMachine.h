#ifndef STATEMACHINE_H
#define STATEMACHINE_H
#include "Messages.h"
#include "Song.h"
#include "SystemException.h"
#define MAX_LOGINS 5
class StateMachine {
public:
  StateMachine();
  ~StateMachine();
  int getClient(int index);
  int getClientNumber(int socketDescriptor);
  bool insertClient(int socketDescriptor);
  bool removeClient(int index);
  ControlMessage *processMessage(uint8_t *buffer, int bufferSize, int sender);
private:
  uint32_t createSong(uint32_t tempo, int clientID);
  bool createTrack(uint8_t *waveType, uint8_t *volume, uint32_t *lfo,
                   uint32_t *depth, uint32_t *detune, uint32_t *rate,
                   uint8_t *numTracks, int clientID);
  bool addNote(uint32_t *note, uint32_t *octave, uint64_t *duration,
               int clientID);
  bool addRest(uint64_t *duration, int clientID);
  bool setGain(int64_t *gain, int clientID);
  bool addDelay(uint32_t *mix, uint32_t *delay, uint32_t *beatDelay,
                uint32_t *feedback, uint32_t *priority, int clientID);
  bool addChorus(uint32_t *mix, uint32_t *delay, uint32_t *depth,
                 uint32_t *rate, uint32_t *priority, int clientID);
  bool addTremolo(uint32_t *mix, uint32_t *rate, uint32_t *mode,
                  uint32_t *priority, int clientID);
  bool addEqualizerBand(uint16_t *frequency, int16_t *gain, uint16_t *bandWidth,
                        uint32_t *priority, int clientID);
  bool addEnvelopeShaper(uint32_t *attack, uint32_t *release,
                         uint32_t *priority, int clientID);
  bool addCompressor(uint32_t *attack, uint32_t *release, uint32_t *ratio,
                     uint32_t *threshold, uint32_t *priority, int clientID);
  bool removeEffect(uint32_t *priority, int clientID);
  bool getSongInfo(uint32_t *numTracks, uint32_t *lenSamples,
                   uint32_t *lenMilis, const char **name, int clientID);
  bool clearSong(int clientNo);
  bool setSongName(const char **name, int clientNo);
  size_t bounceTrack(uint64_t clearSong, uint8_t **buffer, int clientID);
  bool saveSongState(uint8_t **buffer, size_t *size, int clientID);
  bool setTrack(uint8_t *track, int clientID);
  void setLogin(uint8_t loginType, int clientID);
  uint8_t getLogin(int clientID);
  void setCurrentTrack(uint8_t track, int clientID);
  uint8_t getCurrentTrack(int clientID);
  void setNumTracks(uint8_t num, int clientID);
  uint8_t getNumTracks(int clientID);
  bool validatePassword(const char *password);
  const char *getFlag();
  int *clients;
  uint8_t *logins;
  uint8_t *currentTrack;
  uint8_t *numTracks;
  Song *songs[MAX_LOGINS];
  char *flag;
};
#endif