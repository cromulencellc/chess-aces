#include "IDs.h"
#include "SawWave.h"
#include "SinWave.h"
#include "SquareWave.h"
#include "TriangleWave.h"
#include "WaveFactory.h"
#include "WhiteNoise.h"
WaveFactory::WaveFactory() : sampleRate(0) {}
WaveFactory::WaveFactory(uint32_t sampleRate) : sampleRate(sampleRate) {}
Wave *WaveFactory::getWave(WaveType waveType) {
  Wave *retVal = 0;
  switch (waveType) {
  case SINE:
    retVal = new SinWave(this->sampleRate);
    break;
  case SQUARE:
    retVal = new SquareWave(this->sampleRate);
    break;
  case SAW:
    retVal = new SawWave(this->sampleRate);
    break;
  case TRIANGLE:
    retVal = new TriangleWave(this->sampleRate);
    break;
  case WHITE:
    retVal = new WhiteNoise(this->sampleRate);
  }
  return retVal;
}
std::istream &WaveFactory::deserializeWave(std::istream &in, Wave **wave) {
  waveID wid;
  Wave *w = NULL;
  std::string errorMessage = "Detected EOF early in wave detection\n";
  ISTREAM_READ(in, wid, errorMessage);
  switch (wid) {
  case SIN_I: {
    w = new SinWave();
    ISTREAM_READ(in, *w, errorMessage);
    *wave = w;
  } break;
  case SQU_I: {
    w = new SquareWave();
    ISTREAM_READ(in, *w, errorMessage);
    *wave = w;
  } break;
  case SAW_I: {
    w = new SawWave();
    ISTREAM_READ(in, *w, errorMessage);
    *wave = w;
  } break;
  case TRI_I: {
    w = new TriangleWave();
    ISTREAM_READ(in, *w, errorMessage);
    *wave = w;
  } break;
  case WHI_I: {
    w = new WhiteNoise();
    ISTREAM_READ(in, *w, errorMessage);
    *wave = w;
  } break;
  default: {
    SystemException e("Unknow wave type in stream\n");
    throw e;
  } break;
  }
  return in;
}