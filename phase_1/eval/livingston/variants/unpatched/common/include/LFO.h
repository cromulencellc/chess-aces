#ifndef LFO_H
#define LFO_H
#include "Serializable.h"
#include "SinWave.h"
#include "Utils.h"
class LFO : public Serializable {
public:
  LFO();
  LFO(double detune, double modDepth, double rate, uint32_t sampleRate);
  virtual ~LFO();
  double modulateFrequency(double frequencyIn);
protected:
  virtual std::ostream &serialize(std::ostream &out) const;
  virtual std::istream &dserialize(std::istream &in);
private:
  double detune;
  double modDepth;
  double rate;
  SinWave carrier;
};
#endif