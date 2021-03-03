#ifndef UTILS_H
#define UTILS_H
#include <cfloat>
#include <stdint.h>
#define PI 3.14159265358979323846264338327950
typedef double audio_sample;
#define AUDIO_SAMPLE_MAX (DBL_MAX)
typedef int16_t pcm_data;
#define PCM_DATA_MAX (32767)
#define PCM_DATA_MIN (-32768)
audio_sample addSamples(audio_sample a, audio_sample b);
pcm_data addSamples(pcm_data a, pcm_data b);
pcm_data convertSample(audio_sample sample);
double dBToA(double db);
double aToDB(double a);
uint16_t swapEndianness(uint16_t val);
uint32_t swapEndianness(uint32_t val);
#endif