#ifndef TREMOLO_EFFECT_H
#define TREMOLO_EFFECT_H
#define MAX_TREMOLO_RATE (10000)
#define MAX_TREMOLO_MODE_MAX (3)
#include "Effect.h"
#include "Tempo.h"
class TremoloEffect : public Effect {
public:
  TremoloEffect(Tempo &tempo);
  TremoloEffect(int priority, Tempo &tempo, uint32_t mix, uint32_t rate,
                uint32_t mode);
  virtual ~TremoloEffect();
  virtual void applyEffect(std::deque<audio_sample> &dry);
protected:
  virtual std::ostream &serialize(std::ostream &out) const;
  virtual std::istream &dserialize(std::istream &in);
private:
  Tempo &tempo;
  uint32_t mix;
  uint32_t rate;
  uint32_t mode;
};
#endif