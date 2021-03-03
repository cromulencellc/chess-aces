#include "EqualizerEffect.h"
#include "IDs.h"
#include <algorithm>
#include <cmath>
EqualizerEffect::Band::Band() : frequency(0) {
  this->aCoefs[0] = 0;
  this->aCoefs[1] = 0;
  this->aCoefs[2] = 0;
  this->bCoefs[0] = 0;
  this->bCoefs[1] = 0;
  this->bCoefs[2] = 0;
}
EqualizerEffect::Band::Band(double frequency, double gain, double bandWidth,
                            uint32_t sampleRate)
    : frequency(frequency) {
  double amp = ::pow(10, gain / 40.00);
  double omega = (2 * PI * frequency) / sampleRate;
  double alpha = ::sin(omega) / (2.0 * bandWidth);
  this->bCoefs[0] = 1 + (alpha * amp);
  this->bCoefs[1] = -2 * ::cos(omega);
  this->bCoefs[2] = 1 - (alpha * amp);
  this->aCoefs[0] = 1 + (alpha / amp);
  this->aCoefs[1] = -2 * ::cos(omega);
  this->aCoefs[2] = 1 - (alpha / amp);
}
EqualizerEffect::Band::~Band() {}
void EqualizerEffect::Band::applyFilter(std::deque<audio_sample> &dry) {
  audio_sample xVals[3];
  audio_sample yVals[3];
  for (int x = 0; x < 3; ++x) {
    xVals[x] = 0;
    yVals[x] = 0;
  }
  for (int index = 0; index < dry.size(); ++index) {
    xVals[0] = dry[index];
    yVals[0] = ((this->bCoefs[0] * xVals[0]) + (this->bCoefs[1] * xVals[1]) +
                (this->bCoefs[2] * xVals[2]) - (this->aCoefs[1] * yVals[1]) -
                (this->aCoefs[2] * yVals[2])) /
               this->aCoefs[0];
    xVals[2] = xVals[1];
    xVals[1] = xVals[0];
    yVals[2] = yVals[1];
    yVals[1] = yVals[0];
    dry[index] = yVals[0];
  }
}
std::ostream &EqualizerEffect::Band::serialize(std::ostream &out) const {
  out << this->frequency;
  out << this->aCoefs[0];
  out << this->aCoefs[1];
  out << this->aCoefs[2];
  out << this->bCoefs[0];
  out << this->bCoefs[1];
  out << this->bCoefs[2];
  return out;
}
std::istream &EqualizerEffect::Band::dserialize(std::istream &in) {
  std::string errorMessage =
      "Detected EOF early while constructing an equlizer band\n";
  ISTREAM_READ(in, this->frequency, errorMessage);
  ISTREAM_READ(in, this->aCoefs[0], errorMessage);
  ISTREAM_READ(in, this->aCoefs[1], errorMessage);
  ISTREAM_READ(in, this->aCoefs[2], errorMessage);
  ISTREAM_READ(in, this->bCoefs[0], errorMessage);
  ISTREAM_READ(in, this->bCoefs[1], errorMessage);
  ISTREAM_READ(in, this->bCoefs[2], errorMessage);
  return in;
}
EqualizerEffect::EqualizerEffect(Tempo &tempo)
    : tempo(tempo), bands(MAX_EQ_BANDS) {}
EqualizerEffect::EqualizerEffect(int priority, Tempo &tempo)
    : Effect(priority), tempo(tempo), bands(MAX_EQ_BANDS) {}
EqualizerEffect::~EqualizerEffect() {}
bool EqualizerEffect::addBand(double frequency, double gain, double bandWidth) {
  bool retVal = false;
  if ((gain <= 12 && gain >= -12) && bandWidth != 0 &&
      this->bands.size() < MAX_EQ_BANDS) {
    for (auto it = this->bands.begin(); it != this->bands.end();) {
      if ((*it).getFrequency() == frequency) {
        it = this->bands.erase(it);
      } else {
        ++it;
      }
    }
    this->bands.push_back(
        Band(frequency, gain, bandWidth, this->tempo.getSampleRate()));
    retVal = true;
  }
  return retVal;
}
bool EqualizerEffect::removeBand(double frequency) {
  bool retVal = false;
  for (auto it = this->bands.begin(); it != this->bands.end();) {
    if ((*it).getFrequency() == frequency) {
      it = this->bands.erase(it);
      retVal = true;
    } else {
      ++it;
    }
  }
  return retVal;
}
void EqualizerEffect::applyEffect(std::deque<audio_sample> &dry) {
  std::sort(this->bands.begin(), this->bands.end(), [](Band &a, Band &b) {
    return a.getFrequency() < b.getFrequency();
  });
  for (int index = 0; index < this->bands.size(); ++index) {
    this->bands[index].applyFilter(dry);
  }
}
std::ostream &EqualizerEffect::serialize(std::ostream &out) const {
  out << EFFECT_C;
  out << EQ_C;
  std::ostream &o = Effect::serialize(out);
  uint32_t numBands = this->bands.size();
  o << numBands;
  for (auto it = this->bands.begin(); it != bands.end(); ++it) {
    o << *it;
  }
  return o;
}
std::istream &EqualizerEffect::dserialize(std::istream &in) {
  uint32_t numBands;
  std::istream &i = Effect::dserialize(in);
  std::string errorMessage = "Detected EOF early in Envelope Shaper effect\n";
  ISTREAM_READ(i, numBands, errorMessage);
  for (int x = 0; x < numBands; ++x) {
    Band newBand;
    ISTREAM_READ(i, newBand, errorMessage);
  }
  return i;
}
