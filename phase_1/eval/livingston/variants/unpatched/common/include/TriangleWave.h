#ifndef TRIANGLEWAVE_H
#define TRIANGLEWAVE_H
#include "Wave.h"
class TriangleWave : public Wave {
public:
  TriangleWave();
  TriangleWave(uint32_t sampleRate);
  virtual ~TriangleWave();
  virtual double getCarrierSignal(double freq);
  virtual audio_sample getSample(double freq);
protected:
  virtual std::ostream &serialize(std::ostream &out) const;
  virtual std::istream &dserialize(std::istream &in);
private:
  double curvePos;
};
#endif