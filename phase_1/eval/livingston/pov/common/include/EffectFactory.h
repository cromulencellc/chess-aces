#ifndef EFFECT_FACTORY_H
#define EFFECT_FACTORY_H

#include "Serializable.h"
#include "Track.h"

/**
 * Deserializes an effect for the track. If the effect 
 * is an equalizer, it is added to the track as an EQ.
 * 
 * @param in The stream.
 * @param track The track
 * @param tempo The tempo
 * @param numEffects The number of effects.
 * @return the stream
 */
std::istream &deserializeEffectsForTrack(std::istream &in, Track &track, Tempo &tempo, uint32_t numEffects);

#endif 