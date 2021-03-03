#ifndef SINWAVE_H
#define SINWAVE_H
#include "Wave.h"
class SinWave : public Wave {
public:
  SinWave();
  SinWave(uint32_t sampleRate);
  virtual ~SinWave();
  virtual double getCarrierSignal(double freq);
  virtual audio_sample getSample(double freq);
protected:
  virtual std::ostream &serialize(std::ostream &out) const;
  virtual std::istream &dserialize(std::istream &in);
private:
  double curvePos;
};
#endif