#ifndef EQUALIZER_EFFECT_H
#define EQUALIZER_EFFECT_H

#include "Effect.h"
#include "Tempo.h"

#define MAX_EQ_BANDS (7)

class EqualizerEffect: public Effect
{
public:
    /**
     * Constructor
     * 
     * @param tempo The tempo
     */
    EqualizerEffect(Tempo& tempo);

    /**
     * Constructs an equalizer
     * 
     * @param priority The priority of the effect
     * @param tempo The tempo of the song
     */
    EqualizerEffect(int priority, Tempo& tempo);

    /**
     * Destructor 
     */
    virtual ~EqualizerEffect();

    /**
     * Adds a band to the equalizer. If the band has been set up
     * already, the previous settings will be cleared.
     * 
     * @param frequency The center frequency to adjust.
     * @param gain The boost or cut
     * @param bandWidth The width of the band to edit
     * @return True if the band was added.
     */
    bool addBand(double frequency, double gain, double bandWidth);

    /**
     * Creates a new buffer with the effect applied.
     * 
     * @param dry The dry signal
     */
    virtual void applyEffect(std::deque<audio_sample> &dry);

    /**
     * Removes the band from the equalizer.
     * 
     * @param frequency The frequency to remove.
     * @return True if the band was removed.
     */
    bool removeBand(double frequency);

protected:
    /**
     * Serializes the object to the out stream.
     * 
     * @param out The out stream.
     * @return The out stream.
     */
    virtual std::ostream &serialize(std::ostream &out) const;

    /**
     * Deserializes the object from istream.
     *
     * @param in The input stream
     * @return The input stream
     */
    virtual std::istream &dserialize(std::istream &in);

private:

    class Band: public Serializable
    {
    public:
        /**
         * Constructor
         */
        Band();

        /**
         * Constructs a band.
         * 
         * @param frequency The frequency to adjust
         * @param gain The cut or boost value
         * @param bandWidth The width around the frequency to adjust.
         * @param sampleRate
         */ 
        Band(double frequency, double gain, double bandWidth, uint32_t sampleRate);

        /**
         * Destructor
         */
        ~Band();

        /**
         * Applies the filter for the individual band to the 
         * buffer.
         * 
         * @param dry The buffer to affect.
         */
        void applyFilter(std::deque<audio_sample> &dry);

        /**
         * Gets the frequency associated with this band.
         * 
         * @return The frequency associated with this band.
         */
        double getFrequency() { return this->frequency; }

    protected:
        /**
         * Serializes the object to the out stream.
         * 
         * @param out The out stream.
         * @return The out stream.
         */
        virtual std::ostream &serialize(std::ostream &out) const;

        /**
         * Deserializes the object from istream.
         *
         * @param in The input stream
         * @return The input stream
         */
        virtual std::istream &dserialize(std::istream &in);

    private:
        double frequency;
        double aCoefs[3];
        double bCoefs[3];
    };

    Tempo& tempo;
    std::deque<Band> bands;
};

#endif