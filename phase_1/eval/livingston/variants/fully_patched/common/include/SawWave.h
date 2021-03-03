#ifndef SAWWAVE_H
#define SAWWAVE_H
#include "Wave.h"
class SawWave : public Wave {
public:
  SawWave();
  SawWave(uint32_t sampleRate);
  virtual ~SawWave();
  virtual double getCarrierSignal(double freq);
  virtual audio_sample getSample(double freq);
protected:
  virtual std::ostream &serialize(std::ostream &out) const;
  virtual std::istream &dserialize(std::istream &in);
private:
  double curvePos;
};
#endif