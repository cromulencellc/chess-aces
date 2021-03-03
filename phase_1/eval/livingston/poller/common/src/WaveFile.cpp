#include "WaveFile.h"

#include <cstring>

WaveFile::WaveFile(uint32_t sampleRate): 
  bitsPerSample(0x10), 
  dataSize(0x00), 
  fmtDataLength(0x10), 
  format(0x01), 
  numChannels(0x01), 
  sampleRate(sampleRate)
{

}

WaveFile::WaveFile(std::string filePath) 
{

}

bool WaveFile::addPCMData(pcm_data data) 
{
    this->pcmData.push_back(data);
    this->dataSize += sizeof(pcm_data);
    return true;
}

uint16_t WaveFile::getBlockAlign()
{
    return this->numChannels * this->bitsPerSample / 8;
}

uint32_t WaveFile::getByteRate()
{
    return this->sampleRate * this->numChannels * this->bitsPerSample / 8;
}

bool WaveFile::writeDataBuffer(std::ostream &buffer)
{
    // For header information, see:
    // http://www.topherlee.com/software/pcm-tut-wavformat.html

    // ASCII Headers
    char sig[4] = {0x52, 0x49, 0x46, 0x46}; //RIFF
    char format[4] = {0x57, 0x41, 0x56, 0x45}; //WAVE
    char fmt[4] = {0x66, 0x6d, 0x74, 0x20}; //'fmt '
    char data[4] = {0x64, 0x61, 0x74, 0x61}; //DATA

    // Temp variables
    uint16_t val16 = 0;
    uint32_t val32 = 0;

    // Write the signature
    buffer.write( sig, sizeof(sig));

    // Write the file size
    val32 = this->getChunkSize();
    buffer.write((char *) &val32, sizeof(val32));

    // Write the format
    buffer.write( format, sizeof(format));
    buffer.write( fmt, sizeof(fmt));

    // Write format data length
    val32 = this->getFmtDataLength();
    buffer.write((char *) &val32, sizeof(val32));

    // Write the format
    val16 = this->getFormat();
    buffer.write((char *) &val16, sizeof(val16));

    // Write the number of cannels
    val16 = this->getNumChannels();
    buffer.write((char *) &val16, sizeof(val16));

    // Write the sample rate
    val32 = this->getSampleRate();
    buffer.write((char *) &val32, sizeof(val32));

    // Write the byte rate
    val32 = this->getByteRate();
    buffer.write((char *) &val32, sizeof(val32));

    // Write the block align
    val16 = this->getBlockAlign();
    buffer.write((char *) &val16, sizeof(val16));

    // Write the bits per sample
    val16 = this->getBitsPerSample();
    buffer.write((char *) &val16, sizeof(val16));

    // Write the data header
    buffer.write( data, sizeof(data));

    // Write the data length
    val32 = this->getDataSize();
    buffer.write((char *) &val32, sizeof(val32));

    // Add actual sound data
    for (auto pcmDataIt = this->pcmData.begin(); pcmDataIt != this->pcmData.end(); ++pcmDataIt)
    {
        buffer.write( (char *) &(*pcmDataIt), sizeof(pcm_data)/sizeof(char));
    }

    // All done
    return true;         
}

size_t WaveFile::writeDataBuffer(uint8_t **buffer)
{
    size_t retVal = 0;
    *buffer = (uint8_t *) malloc(this->getChunkSize());

    uint8_t *bufferEnd = *buffer + this->getChunkSize();

    if (*buffer)
    {
        retVal = this->getChunkSize();
        uint8_t *buf = *buffer;

        // For header information, see:
        // http://www.topherlee.com/software/pcm-tut-wavformat.html

        // ASCII Headers
        char sig[4] = {0x52, 0x49, 0x46, 0x46}; //RIFF
        char format[4] = {0x57, 0x41, 0x56, 0x45}; //WAVE
        char fmt[4] = {0x66, 0x6d, 0x74, 0x20}; //'fmt '
        char data[4] = {0x64, 0x61, 0x74, 0x61}; //DATA

        // Temp variables
        uint16_t val16 = 0;
        uint32_t val32 = 0;

        // Write the signature
        memcpy(buf, sig, sizeof(sig));
        buf += sizeof(sig);

        // Write the file size
        val32 = this->getChunkSize();
        memcpy(buf, (char *) &val32, sizeof(val32));
        buf += sizeof(val32);

        // Write the format
        memcpy(buf, format, sizeof(format));
        buf += sizeof(format);
        memcpy(buf, fmt, sizeof(fmt));
        buf += sizeof(fmt);

        // Write format data length
        val32 = this->getFmtDataLength();
        memcpy(buf, (char *) &val32, sizeof(val32));
        buf += sizeof(val32);

        // Write the format
        val16 = this->getFormat();
        memcpy(buf, (char *) &val16, sizeof(val16));
        buf += sizeof(val16);

        // Write the number of cannels
        val16 = this->getNumChannels();
        memcpy(buf, (char *) &val16, sizeof(val16));
        buf += sizeof(val16);

        // Write the sample rate
        val32 = this->getSampleRate();
        memcpy(buf, (char *) &val32, sizeof(val32));
        buf += sizeof(val32);

        // Write the byte rate
        val32 = this->getByteRate();
        memcpy(buf, (char *) &val32, sizeof(val32));
        buf += sizeof(val32);

        // Write the block align
        val16 = this->getBlockAlign();
        memcpy(buf, (char *) &val16, sizeof(val16));
        buf += sizeof(val16);

        // Write the bits per sample
        val16 = this->getBitsPerSample();
        memcpy(buf, (char *) &val16, sizeof(val16));
        buf += sizeof(val16);

        // Write the data header
        memcpy(buf, data, sizeof(data));
        buf += sizeof(sizeof(data));

        // Write the data length
        val32 = this->getDataSize();
        memcpy(buf, (char *) &val32, sizeof(val32));
        buf += sizeof(val32);

        // Add actual sound data
        for (auto pcmDataIt = this->pcmData.begin(); pcmDataIt != this->pcmData.end() && buf != bufferEnd; ++pcmDataIt)
        {
            memcpy(buf, (char *) &(*pcmDataIt), sizeof(pcm_data)/sizeof(char));
            buf += sizeof(pcm_data)/sizeof(char);
        }
    }
    // All done
    return retVal;  
}

bool WaveFile::writeFile(std::string filePath) 
{
    bool retVal = true;

    // File setup
    std::ofstream wav;
    wav.open(filePath, std::ios::binary | std::ios::out);
    retVal &= this->writeDataBuffer(wav);
    
    // Close the file
    wav.close();

    // All done
    return true;
}

