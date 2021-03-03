#ifndef TRACK_H
#define TRACK_H
#include "Effect.h"
#include "EqualizerEffect.h"
#include "Note.h"
#include "Serializable.h"
#include "Tempo.h"
#include "Wave.h"
#include <deque>
#define MAX_EFFECTS (10)
#define MAX_TRACK_BOOST (100)
#define MAX_TRACK_CUT (-100)
class Track : public Serializable {
public:
  Track(Tempo &tempo);
  Track(Wave *wave, Tempo &tempo, double volume);
  ~Track();
  void addLFO(double detune, double modDepth, double rate);
  void addNote(Semitone tone, int octave, Tempo::Beat beat);
  void addRest(Tempo::Beat beat);
  bool addEffect(Effect *effect);
  void applyEffects();
  std::deque<audio_sample> &getEffectedBuffer();
  EqualizerEffect *getEqualizer() { return this->eq; };
  double getVolume() { return this->volume; }
  bool setEqualizer(EqualizerEffect *equalizer);
  void setVolume(double volume) { this->volume = volume; }
  Effect *removeEffect(int priority);
  uint64_t getLengthInSamples();
  uint64_t getLengthInMillis();
protected:
  virtual std::ostream &serialize(std::ostream &out) const;
  virtual std::istream &dserialize(std::istream &in);
private:
  std::deque<audio_sample> &getBuffer() { return this->buffer; }
  std::deque<audio_sample> &getDirtyBuffer() { return this->dirtyBuffer; }
  Wave *wave;
  Tempo &tempo;
  double volume;
  Effect *effects[MAX_EFFECTS];
  EqualizerEffect *eq;
  std::deque<audio_sample> buffer;
  std::deque<audio_sample> dirtyBuffer;
};
#endif