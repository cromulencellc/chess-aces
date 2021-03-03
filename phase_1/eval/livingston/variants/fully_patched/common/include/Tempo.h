#ifndef TEMPO_H
#define TEMPO_H
#include "Serializable.h"
#include "Utils.h"
#include <iostream>
#define TEMPO_MAX (1000)
#define TEMPO_MIN (10)
#define BEAT_MAX (Tempo::DOTTED_THIRTYSECOND)
#define BEAT_DEFAULT (Tempo::QUARTER)
class Tempo : public Serializable {
public:
  enum Beat {
    WHOLE,
    HALF,
    QUARTER,
    EIGHTH,
    SIXTEENTH,
    THIRTYSECOND,
    DOTTED_WHOLE,
    DOTTED_HALF,
    DOTTED_QUARTER,
    DOTTED_EIGHTH,
    DOTTED_SIXTEENTH,
    DOTTED_THIRTYSECOND
  };
  Tempo();
  Tempo(uint32_t sampleRate, uint32_t bpm);
  uint32_t getNumberOfSamples(Beat beat);
  uint32_t getNumberOfSamples(uint32_t msec);
  uint32_t getSampleRate() { return this->sampleRate; }
protected:
  virtual std::ostream &serialize(std::ostream &out) const;
  virtual std::istream &dserialize(std::istream &in);
private:
  uint32_t bpm;
  uint32_t sampleRate;
};
#endif