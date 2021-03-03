#ifndef WAVEFACTORY_H
#define WAVEFACTORY_H
#include "Wave.h"
#define MAX_WAVE_TYPE (WaveFactory::WHITE)
class WaveFactory {
public:
  enum WaveType { SINE, SQUARE, SAW, TRIANGLE, WHITE };
  WaveFactory();
  WaveFactory(uint32_t sampleRate);
  Wave *getWave(WaveType waveType);
  std::istream &deserializeWave(std::istream &in, Wave **wave);
  void setSampleRate(uint32_t sampleRate) { this->sampleRate = sampleRate; }
private:
  uint32_t sampleRate;
};
#endif