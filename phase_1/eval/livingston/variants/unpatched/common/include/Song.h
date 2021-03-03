#ifndef SONG_H
#define SONG_H
#include "Serializable.h"
#include "Track.h"
#include "WaveFactory.h"
#include "WaveFile.h"
#include <deque>
#include <string>
class Song : public Serializable {
public:
  Song();
  Song(uint32_t sampleRate, uint32_t bpm, std::string name);
  ~Song();
  void addNote(uint32_t trackNumber, Semitone note, int octave,
               Tempo::Beat duration);
  void addRest(uint32_t trackNumber, Tempo::Beat duration);
  void addEffect(uint32_t trackNumber, Effect *effect);
  void addEqualizer(uint32_t trackNumber, EqualizerEffect *eq);
  uint32_t addTrack(WaveFactory::WaveType waveType, double volume);
  void addLFO(uint32_t trackNumber, double detune, double modDepth,
              double rate);
  void bounceToBuffer(std::ostream &buffer);
  size_t bounceToBuffer(uint8_t **buffer);
  void bounceToFile();
  EqualizerEffect *getEqualizer(uint32_t trackNumber);
  uint32_t getLengthSamples();
  uint32_t getLengthMillis();
  uint32_t getNumTracks() { return this->tracks.size(); }
  std::string &getSongName() { return this->name; }
  Tempo &getTempo() { return this->tempo; }
  void removeEffect(uint32_t trackNumber, int priority);
  void setVolume(uint32_t trackNumber, int64_t gain);
  void setSongName(std::string &name) { this->name = name; }
  uint64_t getLengthInSamples();
protected:
  virtual std::ostream &serialize(std::ostream &out) const;
  virtual std::istream &dserialize(std::istream &in);
private:
  void generateWaveData(WaveFile &file);
  WaveFactory factory;
  std::string name;
  uint32_t sampleRate;
  Tempo tempo;
  std::deque<Track *> tracks;
};
#endif