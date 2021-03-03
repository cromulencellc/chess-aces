#ifndef SQUAREWAVE_H
#define SQUAREWAVE_H
#include "Wave.h"
class SquareWave : public Wave {
public:
  SquareWave();
  SquareWave(uint32_t sampleRate);
  virtual ~SquareWave();
  virtual double getCarrierSignal(double freq);
  virtual audio_sample getSample(double freq);
protected:
  virtual std::ostream &serialize(std::ostream &out) const;
  virtual std::istream &dserialize(std::istream &in);
private:
  double curvePos;
};
#endif