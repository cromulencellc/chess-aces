#include "EnvelopeShaperEffect.h"
#include "IDs.h"
#include <cmath>
EnvelopeShaperEffect::EnvelopeShaperEffect(Tempo &tempo)
    : Effect(), tempo(tempo), envelope(0), attack(0), release(0) {}
EnvelopeShaperEffect::EnvelopeShaperEffect(int priority, Tempo &tempo,
                                           uint32_t attack, uint32_t release)
    : Effect(priority), tempo(tempo), envelope(0.00) {
  this->attack = this->convert((double)attack);
  this->release = this->convert((double)release);
}
EnvelopeShaperEffect::~EnvelopeShaperEffect() {}
void EnvelopeShaperEffect::applyEffect(std::deque<audio_sample> &dry) {
  for (int index = 0; index < dry.size(); ++index) {
    dry[index] = (audio_sample) this->processSample((double)dry[index]);
  }
}
double EnvelopeShaperEffect::processSample(double sample) {
  double retVal = sample;
  if (sample > this->envelope) {
    retVal = this->attack * (sample - this->envelope);
  } else if (sample < this->envelope) {
    retVal = this->release * (sample - this->envelope);
  }
  return retVal;
}
void EnvelopeShaperEffect::setAttack(double attack) {
  this->attack = this->convert(attack);
}
void EnvelopeShaperEffect::setRelease(double release) {
  this->release = this->convert(release);
}
double EnvelopeShaperEffect::convert(double value) {
  double retVal = 1;
  if (value > 0) {
    retVal =
        1.00 - ::exp(-1.00 / (value * 0.001 * this->tempo.getSampleRate()));
  }
  return retVal;
}
std::ostream &EnvelopeShaperEffect::serialize(std::ostream &out) const {
  out << EFFECT_C;
  out << ENVELOPE_C;
  std::ostream &o = Effect::serialize(out);
  o << this->attack;
  o << this->release;
  o << this->envelope;
  return o;
}
std::istream &EnvelopeShaperEffect::dserialize(std::istream &in) {
  std::istream &i = Effect::dserialize(in);
  std::string errorMessage = "Detected EOF early in Envelope Shaper effect\n";
  ISTREAM_READ(i, this->attack, errorMessage);
  ISTREAM_READ(i, this->release, errorMessage);
  ISTREAM_READ(i, this->envelope, errorMessage);
  return i;
}