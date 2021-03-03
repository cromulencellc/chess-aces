#include "IDs.h"
#include "SawWave.h"
#include <iostream>
#include <math.h>
SawWave::SawWave() : Wave(), curvePos(0) {}
SawWave::SawWave(uint32_t sampleRate) : Wave(sampleRate), curvePos(0) {}
SawWave::~SawWave() {}
double SawWave::getCarrierSignal(double freq) {
  double adjustedFreq = this->getOscilatedFrequency(freq);
  this->curvePos += adjustedFreq / this->getSampleRate();
  double raw = this->curvePos - ::floor(this->curvePos);
  return raw;
}
audio_sample SawWave::getSample(double freq) {
  double adjustedFreq = this->getOscilatedFrequency(freq);
  this->curvePos += adjustedFreq / this->getSampleRate();
  double raw = this->curvePos - ::floor(this->curvePos);
  return (audio_sample)(raw);
}
std::ostream &SawWave::serialize(std::ostream &out) const {
  out << SAW_C;
  std::ostream &o = Wave::serialize(out);
  o << this->curvePos;
  return o;
}
std::istream &SawWave::dserialize(std::istream &in) {
  std::string errorMessage = "Detected EOF early in SawWave\n";
  std::istream &i = Wave::dserialize(in);
  ISTREAM_READ(i, this->curvePos, errorMessage);
  return i;
}