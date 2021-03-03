#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <cfloat>

/**
 * Pi
 */
#define PI 3.14159265358979323846264338327950

/**
 * The type being used as an audio sample
 */
typedef double audio_sample;

/**
 * The max size of an audio sample
 */
#define AUDIO_SAMPLE_MAX (DBL_MAX) 

/**
 * The type of data being written to file
 */
typedef int16_t pcm_data;

/**
 * The max valur for PCM data.
 */
#define PCM_DATA_MAX (32767)

/**
 * The min value for PCM data.
 */
#define PCM_DATA_MIN (-32768)

/**
 * Combines two samples and returns the resulatant sample.
 * 
 * @param a A sample
 * @param b A sample
 * @return The combination of the signals.
 */
audio_sample addSamples(audio_sample a, audio_sample b);

/**
 * Combines two samples and returns the resulatant sample.
 * 
 * @param a A sample
 * @param b A sample
 * @return The combination of the signals.
 */
pcm_data addSamples(pcm_data a, pcm_data b);

/**
 * Converts the sample to PCM data.
 * 
 * @param sample The sample to convert
 * @return pcm data
 */
pcm_data convertSample(audio_sample sample);

/**
 * Converts values from dB into sample amplitude.
 * 
 * @param db The dB value.
 * @return The amplitude value.
 */
double dBToA(double db);

/**
 * Converts amplitude values to dB values.
 * 
 * @param a The amplitude value
 * @return The dB value.
 */
double aToDB(double a);

/**
 * Swapps endianness of 16 bit values
 * 
 * @param val The value to swap
 * @return The value with endianness swapped.
 */
uint16_t swapEndianness(uint16_t val);

/**
 * Swapps endianness of 32 bit values
 * 
 * @param val The value to swap
 * @return The value with endianness swapped.
 */
uint32_t swapEndianness(uint32_t val);

#endif