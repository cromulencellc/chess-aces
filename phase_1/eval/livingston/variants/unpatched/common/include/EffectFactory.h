#ifndef EFFECT_FACTORY_H
#define EFFECT_FACTORY_H
#include "Serializable.h"
#include "Track.h"
std::istream &deserializeEffectsForTrack(std::istream &in, Track &track,
                                         Tempo &tempo, uint32_t numEffects);
#endif