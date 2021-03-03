#include "Utils.h"
#include <algorithm>
#include <cmath>
audio_sample addSamples(audio_sample a, audio_sample b) {
  return a + b - ((a * b) / AUDIO_SAMPLE_MAX);
}
pcm_data addSamples(pcm_data a, pcm_data b) {
  return a + b - ((a * b) / PCM_DATA_MAX);
}
pcm_data convertSample(audio_sample sample) {
  pcm_data retVal = 0;
  audio_sample s = sample * 32768;
  if (s > PCM_DATA_MAX) {
    s = PCM_DATA_MAX;
  }
  if (s < PCM_DATA_MIN) {
    s = PCM_DATA_MIN;
  }
  retVal = (pcm_data)s;
  return retVal;
}
double dBToA(double db) { return ::pow(10.00, db / 20.00); }
double aToDB(double a) {
  double l = dBToA(-96.00);
  double retVal = std::max(a, l);
  retVal = 20.00 * ::log10(retVal);
  return retVal;
}
uint16_t swapEndianness(uint16_t val) {
  return ((val & 0x00FF) << 8) | ((val & 0xFF00) >> 8);
}
uint32_t swapEndianness(uint32_t val) {
  return ((val & 0x000000FF) << 24) | ((val & 0x0000FF00) << 8) |
         ((val & 0x00FF0000) >> 8) | ((val & 0xFF000000) >> 24);
}