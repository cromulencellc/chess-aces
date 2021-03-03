#ifndef NOTE_H
#define NOTE_H

/**
 * Enum that types semitones
 */
enum Semitone
{
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

/**
 * Returns the real frequency of the given semitone in the 
 * given octave.
 * 
 * @param note The desired semitone
 * @param octave The desired octave
 * @return The frequency of the specified note.
 */
float getFreq(Semitone note, int octave);



#endif