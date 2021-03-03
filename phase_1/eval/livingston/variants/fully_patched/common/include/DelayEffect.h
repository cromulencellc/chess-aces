#ifndef DELAY_EFFECT_H
#define DELAY_EFFECT_H
#include "Effect.h"
#include "Tempo.h"
#define MAX_DELAY_FEEDBACK (5)
#define MAX_DELAY (1000)
class DelayEffect : public Effect {
public:
  DelayEffect(Tempo &tempo);
  DelayEffect(int priority, Tempo &tempo, uint32_t mix, uint32_t delay,
              Tempo::Beat beatDelay, uint32_t feedback);
  virtual ~DelayEffect();
  virtual void applyEffect(std::deque<audio_sample> &dry);
protected:
  virtual std::ostream &serialize(std::ostream &out);
  virtual std::istream &dserialize(std::istream &in);
private:
  Tempo &tempo;
  uint32_t mix;
  uint32_t delay;
  Tempo::Beat beatDelay;
  uint32_t feedback;
};
#endif