#include "IDs.h"
#include "Song.h"
Song::Song()
    : name(""), sampleRate(0), tempo(Tempo()), factory(WaveFactory()) {}
Song::Song(uint32_t sampleRate, uint32_t bpm, std::string name)
    : name(name), sampleRate(sampleRate), tempo(Tempo(sampleRate, bpm)),
      factory(WaveFactory(sampleRate)) {}
Song::~Song() {
  while (this->tracks.size() > 0) {
    Track *t = this->tracks[this->tracks.size() - 1];
    this->tracks.pop_back();
    delete t;
  }
}
void Song::addNote(uint32_t trackNumber, Semitone note, int octave,
                   Tempo::Beat duration) {
  if (trackNumber < this->tracks.size()) {
    this->tracks[trackNumber]->addNote(note, octave, duration);
  }
}
void Song::addRest(uint32_t trackNumber, Tempo::Beat duration) {
  if (trackNumber < this->tracks.size()) {
    this->tracks[trackNumber]->addRest(duration);
  }
}
void Song::addEffect(uint32_t trackNumber, Effect *effect) {
  if (trackNumber < this->tracks.size()) {
    this->tracks[trackNumber]->addEffect(effect);
  }
}
void Song::addEqualizer(uint32_t trackNumber, EqualizerEffect *eq) {
  if (trackNumber < this->tracks.size()) {
    EqualizerEffect *oldEq = this->tracks[trackNumber]->getEqualizer();
    if (oldEq) {
      this->tracks[trackNumber]->removeEffect(oldEq->getPriority());
    }
    delete oldEq;
    this->tracks[trackNumber]->setEqualizer(eq);
  }
}
EqualizerEffect *Song::getEqualizer(uint32_t trackNumber) {
  EqualizerEffect *retVal = NULL;
  if (trackNumber < this->tracks.size()) {
    retVal = this->tracks[trackNumber]->getEqualizer();
  }
  return retVal;
}
uint32_t Song::addTrack(WaveFactory::WaveType waveType, double volume) {
  uint32_t retVal = this->tracks.size();
  Wave *waveForm = this->factory.getWave(waveType);
  Track *t = new Track(waveForm, this->tempo, volume);
  this->tracks.push_back(t);
  return retVal;
}
void Song::addLFO(uint32_t trackNumber, double detune, double modDepth,
                  double rate) {
  if (trackNumber < this->tracks.size()) {
    this->tracks[trackNumber]->addLFO(detune, modDepth, rate);
  }
}
uint32_t Song::getLengthSamples() {
  uint32_t retVal = 0;
  for (auto it = this->tracks.begin(); it < this->tracks.end(); ++it) {
    if ((*it)->getLengthInSamples() > retVal) {
      retVal = (*it)->getLengthInSamples();
    }
  }
  return retVal;
}
uint32_t Song::getLengthMillis() {
  uint32_t retVal = 0;
  for (auto it = this->tracks.begin(); it < this->tracks.end(); ++it) {
    if ((*it)->getLengthInMillis() > retVal) {
      retVal = (*it)->getLengthInMillis();
    }
  }
  return retVal;
}
uint64_t Song::getLengthInSamples() {
  uint64_t retVal = 0;
  for (auto it = this->tracks.begin(); it != this->tracks.end(); ++it) {
    if ((*it)->getLengthInSamples() > retVal) {
      retVal = (*it)->getLengthInSamples();
    }
  }
  return retVal;
}
void Song::generateWaveData(WaveFile &file) {
  for (int h = 0; h < this->tracks.size(); ++h) {
    this->tracks[h]->applyEffects();
  }
  bool keepGoing = true;
  for (int i = 0; keepGoing; ++i) {
    keepGoing = false;
    audio_sample sample = 0;
    for (int j = 0; j < this->tracks.size(); ++j) {
      std::deque<audio_sample> &trackBuffer =
          this->tracks[j]->getEffectedBuffer();
      if (i < trackBuffer.size()) {
        sample = addSamples(sample, trackBuffer[i]);
        keepGoing = true;
      }
    }
    if (keepGoing) {
      pcm_data out = convertSample(sample);
      file.addPCMData(out);
    }
  }
}
void Song::bounceToBuffer(std::ostream &buffer) {
  WaveFile file(this->sampleRate);
  this->generateWaveData(file);
  file.writeDataBuffer(buffer);
}
size_t Song::bounceToBuffer(uint8_t **buffer) {
  WaveFile file(this->sampleRate);
  this->generateWaveData(file);
  return file.writeDataBuffer(buffer);
}
void Song::bounceToFile() {
  WaveFile file(this->sampleRate);
  this->generateWaveData(file);
  file.writeFile(this->name.append(".wav"));
}
void Song::removeEffect(uint32_t trackNumber, int priority) {
  if (trackNumber < this->tracks.size()) {
    Effect *e = this->tracks[trackNumber]->removeEffect(priority);
    if (e) {
      delete e;
    }
  }
}
void Song::setVolume(uint32_t trackNumber, int64_t gain) {
  if (trackNumber < this->tracks.size()) {
    this->tracks[trackNumber]->setVolume(gain);
  }
}
std::ostream &Song::serialize(std::ostream &out) const {
  out << sampleRate;
  out << name;
  out << tempo;
  uint32_t numTracks = this->tracks.size();
  out << numTracks;
  for (auto it = this->tracks.begin(); it != this->tracks.end(); ++it) {
    out << *(*it);
  }
  return out;
}
std::istream &Song::dserialize(std::istream &in) {
  std::string errorMessage = "Detected EOF early in Song\n";
  ISTREAM_READ(in, this->sampleRate, errorMessage);
  ISTREAM_READ(in, this->name, errorMessage);
  ISTREAM_READ(in, this->tempo, errorMessage);
  this->factory.setSampleRate(this->sampleRate);
  uint32_t numTracks;
  ISTREAM_READ(in, numTracks, errorMessage);
  for (uint32_t x = 0; x < numTracks; ++x) {
    typeID tid;
    ISTREAM_READ(in, tid, errorMessage);
    if (tid == TRACK_I) {
      Track *t = new Track(this->tempo);
      ISTREAM_READ(in, *t, errorMessage);
      this->tracks.push_back(t);
    } else {
      SystemException e("Error in song decoding, expecting a track\n");
    }
  }
  return in;
}