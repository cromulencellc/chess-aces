#include "LFO.h"
#include "Wave.h"
Wave::Wave() : sampleRate(0), lfo(0) {}
Wave::Wave(uint32_t sampleRate) : sampleRate(sampleRate), lfo(0) {}
Wave::~Wave() {
  if (this->lfo) {
    delete this->lfo;
    this->lfo = 0;
  }
}
void Wave::addLFO(double detune, double modDepth, double rate) {
  if (this->lfo) {
    delete this->lfo;
  }
  this->lfo = new LFO(detune, modDepth, rate, this->sampleRate);
}
double Wave::getOscilatedFrequency(double frequencyIn) {
  double retVal = frequencyIn;
  if (this->lfo) {
    retVal = this->lfo->modulateFrequency(frequencyIn);
  }
  return retVal;
}
uint16_t Wave::getSampleRate() { return this->sampleRate; }
std::ostream &Wave::serialize(std::ostream &out) const {
  out << this->sampleRate;
  if (this->lfo) {
    out << true;
    out << this->lfo;
  } else {
    out << false;
  }
  return out;
}
std::istream &Wave::dserialize(std::istream &in) {
  bool hasLfo;
  std::string errorMessage = "Detected EOF early in Wave\n";
  ISTREAM_READ(in, this->sampleRate, errorMessage);
  ISTREAM_READ(in, hasLfo, errorMessage);
  if (hasLfo) {
    LFO *l = new LFO();
    ISTREAM_READ(in, *l, errorMessage);
    this->lfo = l;
  }
  return in;
}