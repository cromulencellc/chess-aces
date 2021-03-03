#ifndef COMPRESSOR_EFFECT_H
#define COMPRESSOR_EFFECT_H
#include "EnvelopeShaperEffect.h"
class CompressorEffect : public EnvelopeShaperEffect {
public:
  CompressorEffect(Tempo &tempo);
  CompressorEffect(int priority, Tempo &tempo, uint32_t attack,
                   uint32_t release, uint32_t ratio, uint32_t threshold);
  ~CompressorEffect();
  virtual void applyEffect(std::deque<audio_sample> &dry);
protected:
  virtual std::ostream &serialize(std::ostream &out) const;
  virtual std::istream &dserialize(std::istream &in);
private:
  audio_sample compressSample(audio_sample sample);
  double ratio;
  double threshold;
};
#endif