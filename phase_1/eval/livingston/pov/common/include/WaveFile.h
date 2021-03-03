#ifndef WAVE_FILE_H
#define WAVE_FILE_H

#include "Utils.h"

#include <string>
#include <fstream>
#include <iostream>
#include <deque>


/**
 * Class to represent a wave file.
 */
class WaveFile 
{
public:
    /**
     * Creates a blank wave file object. This has a few
     * defaults; riff wav, PCM format with 1 channel.
     * 
     * @param sampleRate The sample rate of the file
     */
    WaveFile(uint32_t sampleRate);

    /**
     * Creates a wavefile from a given path.
     * 
     * @param filePath The file path of the wave to load.
     */
    WaveFile(std::string filePath);

    /**
     * Adds sound data in PCM format to the wav file.
     * 
     * @param data The data to add.
     */
    bool addPCMData(pcm_data data);

    /**
     * Writes the wave file content to a buffer.
     * 
     * @param buffer The buffer to fill.
     * @return True on success
     */
    bool writeDataBuffer(std::ostream &buffer);

    /**
     * Writes the wave file content to a buffer.
     * 
     * @param buffer  The buffer to fill.
     * @return The size of the buffer.
     */
    size_t writeDataBuffer(uint8_t **buffer);

    /**
     * Write the wave data to the file specified. This results
     * in the creation of a wave file at the path that contains the
     * data specified by this object.
     * 
     * @param filePath The path to the file to be saved.
     */
    bool writeFile(std::string filePath);

protected:
    /**
     * Returns the number of bits per sample.
     * 
     * @return the number of bits per sample
     */
    uint32_t getBitsPerSample() { return this->bitsPerSample; }

    /**
     * Gets the block align data. This is calculated as
     * NumChannels * BitsPerSample/8
     * 
     * @return block align data.
     */
    uint16_t getBlockAlign();
     
    /**
     * Returns the bytes per sample. This is calculated using the 
     * following formula: SampleRate * NumChannels * BitsPerSample / 8
     * 
     * @return bytes per sample
     */
    uint32_t getByteRate();

    /**
     * Returns the overall file size
     * 
     * @return the chunk size
     */
    uint32_t getChunkSize() { return this->dataSize + 0x2C; }

    /**
     * Returns the size of the data segment of the file.
     * 
     * @return the size of the data segment.
     */
    uint32_t getDataSize() { return this->dataSize; }    

    /**
     * Returns the format data length.
     * 
     * @return the format data length.
     */
    uint32_t getFmtDataLength() { return this->fmtDataLength; }

    /**
     * Gets the format data.
     * 
     * @return format data
     */
    uint16_t getFormat() { return this->format; }

    /**
     * Returns the number of channels.
     * 
     * @return the number of channels
     */
    uint16_t getNumChannels() { return this->numChannels; }

    /**
     * Returns the sample rate.
     * 
     * @return the sample rate.
     */
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


  