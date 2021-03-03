#include "IDs.h"
#include "TriangleWave.h"
#include <iostream>
#include <math.h>
TriangleWave::TriangleWave() : Wave(), curvePos(0) {}
TriangleWave::TriangleWave(uint32_t sampleRate)
    : Wave(sampleRate), curvePos(0) {}
TriangleWave::~TriangleWave() {}
double TriangleWave::getCarrierSignal(double freq) {
  double adjustedFreq = this->getOscilatedFrequency(freq);
  this->curvePos += adjustedFreq / this->getSampleRate();
  double raw = 2 * ::abs(this->curvePos - ::floor(this->curvePos + .5));
  return raw;
}
audio_sample TriangleWave::getSample(double freq) {
  double adjustedFreq = this->getOscilatedFrequency(freq);
  this->curvePos += adjustedFreq / this->getSampleRate();
  double raw = 2 * ::abs(this->curvePos - ::floor(this->curvePos + .5));
  return (audio_sample)raw;
}
std::ostream &TriangleWave::serialize(std::ostream &out) const {
  out << TRI_C;
  std::ostream &o = Wave::serialize(out);
  o << this->curvePos;
  return o;
}
std::istream &TriangleWave::dserialize(std::istream &in) {
  std::string errorMessage = "Detected EOF early in TriangleWave\n";
  std::istream &i = Wave::dserialize(in);
  ISTREAM_READ(i, this->curvePos, errorMessage);
  return i;
}