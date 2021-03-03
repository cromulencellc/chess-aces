#ifndef ENVELOPE_SHAPER_EFFECT_H
#define ENVELOPE_SHAPER_EFFECT_H
#include "Effect.h"
#include "Tempo.h"
class EnvelopeShaperEffect : public Effect {
public:
  EnvelopeShaperEffect(Tempo &tempo);
  EnvelopeShaperEffect(int priority, Tempo &tempo, uint32_t attack,
                       uint32_t release);
  ~EnvelopeShaperEffect();
  virtual void applyEffect(std::deque<audio_sample> &dry);
protected:
  virtual std::ostream &serialize(std::ostream &out) const;
  virtual std::istream &dserialize(std::istream &in);
  double getAttack() { return this->attack; }
  double getRelease() { return this->release; }
  double processSample(double sample);
  void setAttack(double attack);
  void setRelease(double release);
private:
  double convert(double value);
  Tempo &tempo;
  double attack;
  double release;
  double envelope;
};
#endif