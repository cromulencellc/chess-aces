#ifndef WAVE_FILE_H
#define WAVE_FILE_H
#include "Utils.h"
#include <deque>
#include <fstream>
#include <iostream>
#include <string>
class WaveFile {
public:
  WaveFile(uint32_t sampleRate);
  WaveFile(std::string filePath);
  bool addPCMData(pcm_data data);
  bool writeDataBuffer(std::ostream &buffer);
  size_t writeDataBuffer(uint8_t **buffer);
  bool writeFile(std::string filePath);
protected:
  uint32_t getBitsPerSample() { return this->bitsPerSample; }
  uint16_t getBlockAlign();
  uint32_t getByteRate();
  uint32_t getChunkSize() { return this->dataSize + 0x2C; }
  uint32_t getDataSize() { return this->dataSize; }
  uint32_t getFmtDataLength() { return this->fmtDataLength; }
  uint16_t getFormat() { return this->format; }
  uint16_t getNumChannels() { return this->numChannels; }
  uint32_t getSampleRate() { return this->sampleRate; }
private:
  uint16_t bitsPerSample;
  uint32_t dataSize;
  uint32_t fmtDataLength;
  uint16_t format;
  uint16_t numChannels;
  uint32_t sampleRate;
  std::deque<pcm_data> pcmData;
};
#endif
