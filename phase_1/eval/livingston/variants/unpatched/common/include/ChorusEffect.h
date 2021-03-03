#ifndef CHORUS_EFFECT_H
#define CHORUS_EFFECT_H
#define MAX_CHORUS_DELAY (500)
#include "Effect.h"
#include "LFO.h"
#include "Tempo.h"
class ChorusEffect : public Effect {
public:
  ChorusEffect(Tempo &tempo);
  ChorusEffect(int priority, Tempo &tempo, uint32_t mix, uint32_t delay,
               uint32_t depth, uint32_t rate);
  virtual ~ChorusEffect();
  virtual void applyEffect(std::deque<audio_sample> &dry);
protected:
  virtual std::ostream &serialize(std::ostream &out) const;
  virtual std::istream &dserialize(std::istream &in);
private:
  Tempo &tempo;
  uint32_t mix;
  uint32_t delay;
  uint64_t depth;
  LFO lfo;
};
#endif
