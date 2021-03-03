#ifndef WAVE_H
#define WAVE_H
#include "Serializable.h"
#include "Utils.h"
class LFO;
class Wave : public Serializable {
public:
  Wave();
  Wave(uint32_t sampleRate);
  virtual ~Wave();
  void addLFO(double detune, double modDepth, double rate);
  virtual double getCarrierSignal(double freq) = 0;
  virtual audio_sample getSample(double freq) = 0;
protected:
  virtual std::ostream &serialize(std::ostream &out) const;
  virtual std::istream &dserialize(std::istream &in);
protected:
  double getOscilatedFrequency(double frequencyIn);
  uint16_t getSampleRate();
private:
  uint32_t sampleRate;
  LFO *lfo;
};
#endif