#ifndef AUDIO_PARAMETRIC_EQUALIZER
#define AUDIO_PARAMETRIC_EQUALIZER

#include <stdint.h>

typedef struct _frequency_spectrum_descriptor_
{
    float m_SampleRate;     // (Fs) - in HZ - The sample rate of the sequence
    float m_Frequency;      // (f0) - in HZ - The Target frequency on the spetrum
    float m_Bandwidth;      // (Bf) - in HZ - The width/range of frequencies around F0. (low = narrow; high = wide)
    float m_BandwidthGain;  // (GB) - in DB - The level of gain where we are measuring
    float m_ReferenceGain;  // (G0) - in DB - The level of offset of this section
    float m_GainAdjustment; // (G)  - in DB - The level of adjustment applies to the target frequencies.  0 = none
} APE_FrequencySpectrum;

typedef uint32_t APE_EqualizerHandle;
typedef float APE_Sample;

APE_EqualizerHandle ape_obtain();
void ape_return(APE_EqualizerHandle handle);

void ape_run_filter(APE_EqualizerHandle handle, const APE_FrequencySpectrum* frequncy_sample, const APE_Sample* const in_samples, APE_Sample* out_samples, uint32_t num_samples);

#endif