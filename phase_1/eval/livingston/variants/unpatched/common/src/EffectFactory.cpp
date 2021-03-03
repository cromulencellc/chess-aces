#include "ChorusEffect.h"
#include "CompressorEffect.h"
#include "DelayEffect.h"
#include "EffectFactory.h"
#include "EnvelopeShaperEffect.h"
#include "EqualizerEffect.h"
#include "IDs.h"
#include "TremoloEffect.h"
std::istream &deserializeEffectsForTrack(std::istream &in, Track &track,
                                         Tempo &tempo, uint32_t numEffects) {
  std::string errorMessage = "Detected EOF while determining effect\n";
  for (uint32_t x = 0; x < numEffects; ++x) {
    typeID tid;
    ISTREAM_READ(in, tid, errorMessage);
    if (tid == EFFECT_I) {
      effectID eid;
      ISTREAM_READ(in, eid, errorMessage);
      switch (eid) {
      case CHORUS_I: {
        ChorusEffect *e = new ChorusEffect(tempo);
        ISTREAM_READ(in, *e, errorMessage);
        if (!track.addEffect(e)) {
          SystemException e("Error with effect from stream\n");
          throw e;
        }
      } break;
      case COMPRESS_I: {
        CompressorEffect *e = new CompressorEffect(tempo);
        ISTREAM_READ(in, *e, errorMessage);
        if (!track.addEffect(e)) {
          SystemException e("Error with effect from stream\n");
          throw e;
        }
      } break;
      case DELAY_I: {
        DelayEffect *e = new DelayEffect(tempo);
        ISTREAM_READ(in, *e, errorMessage);
        if (!track.addEffect(e)) {
          SystemException e("Error with effect from stream\n");
          throw e;
        }
      } break;
      case ENVELOPE_I: {
        EnvelopeShaperEffect *e = new EnvelopeShaperEffect(tempo);
        ISTREAM_READ(in, *e, errorMessage);
        if (!track.addEffect(e)) {
          SystemException e("Error with effect from stream\n");
          throw e;
        }
      } break;
      case EQ_I: {
        EqualizerEffect *e = new EqualizerEffect(tempo);
        ISTREAM_READ(in, *e, errorMessage);
        if (!track.setEqualizer(e)) {
          SystemException e("Error with effect from stream\n");
        }
      } break;
      case TREMOLO_I: {
        TremoloEffect *e = new TremoloEffect(tempo);
        ISTREAM_READ(in, *e, errorMessage);
        if (!track.addEffect(e)) {
          SystemException e("Error with effect from stream\n");
          throw e;
        }
      } break;
      default: {
        SystemException e("Unknow effect in stream\n");
        throw e;
      } break;
      }
    } else {
      SystemException e("Error decoding effect.\n");
      throw e;
    }
  }
  return in;
}