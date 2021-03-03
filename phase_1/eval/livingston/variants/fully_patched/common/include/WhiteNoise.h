#ifndef WHITENOISE_H
#define WHITENOISE_H
#include "Wave.h"
#include <random>
class WhiteNoise : public Wave {
public:
  WhiteNoise();
  WhiteNoise(uint32_t sampleRate);
  virtual ~WhiteNoise();
  virtual double getCarrierSignal(double freq);
  virtual audio_sample getSample(double freq);
protected:
  virtual std::ostream &serialize(std::ostream &out) const;
  virtual std::istream &dserialize(std::istream &in);
private:
  std::mt19937 gen;
  std::uniform_real_distribution<> dis;
};
#endif