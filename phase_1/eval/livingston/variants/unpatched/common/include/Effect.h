#ifndef EFFECT_H
#define EFFECT_H
#include "Serializable.h"
#include "Utils.h"
#include <deque>
#include <iostream>
class Effect : public Serializable {
public:
  Effect(int priority) { this->priority = priority; }
  Effect() {}
  virtual ~Effect(){};
  virtual void applyEffect(std::deque<audio_sample> &dry) = 0;
  int getPriority() { return this->priority; }
protected:
  virtual std::ostream &serialize(std::ostream &out) const;
  virtual std::istream &dserialize(std::istream &in);
private:
  int priority;
};
#endif