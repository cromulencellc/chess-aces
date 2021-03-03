#ifndef EQUALIZER_EFFECT_H
#define EQUALIZER_EFFECT_H
#include "Effect.h"
#include "Tempo.h"
#define MAX_EQ_BANDS (7)
class EqualizerEffect : public Effect {
public:
  EqualizerEffect(Tempo &tempo);
  EqualizerEffect(int priority, Tempo &tempo);
  virtual ~EqualizerEffect();
  bool addBand(double frequency, double gain, double bandWidth);
  virtual void applyEffect(std::deque<audio_sample> &dry);
  bool removeBand(double frequency);
protected:
  virtual std::ostream &serialize(std::ostream &out) const;
  virtual std::istream &dserialize(std::istream &in);
private:
  class Band : public Serializable {
  public:
    Band();
    Band(double frequency, double gain, double bandWidth, uint32_t sampleRate);
    ~Band();
    void applyFilter(std::deque<audio_sample> &dry);
    double getFrequency() { return this->frequency; }
  protected:
    virtual std::ostream &serialize(std::ostream &out) const;
    virtual std::istream &dserialize(std::istream &in);
  private:
    double frequency;
    double aCoefs[3];
    double bCoefs[3];
  };
  Tempo &tempo;
  std::deque<Band> bands;
};
#endif