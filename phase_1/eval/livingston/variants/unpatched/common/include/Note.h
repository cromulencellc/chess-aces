#ifndef NOTE_H
#define NOTE_H
enum Semitone {
  C = 0,
  CSDF = 1,
  D = 2,
  DSEF = 3,
  E = 4,
  F = 5,
  FSGF = 6,
  G = 7,
  GSAF = 8,
  A = 9,
  ASBF = 10,
  B = 11
};
float getFreq(Semitone note, int octave);
#endif