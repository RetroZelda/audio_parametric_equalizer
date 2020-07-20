#include "audio_parametric_equalizer.h"
#include "array.h"
#include "queue.h"
#include <features.h>
#include <assert.h>
#include <stddef.h>
#include <malloc.h>
#include <string.h>
#include <math.h>

/* Some useful constants. defined in math.h that might not be available to specific systems */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_HISTORY_COUNT 2
typedef struct _parametric_equalizer_data
{
    APE_EqualizerHandle m_Handle;
    APE_FrequencySpectrum m_Spectrum;
    float m_RawSamples[SAMPLE_HISTORY_COUNT];
    float m_ProcessedSamples[SAMPLE_HISTORY_COUNT];
    float m_A0;
    float m_A1;
    float m_A2;
    float m_B0;
    float m_B1;
    float m_B2;
} APE_CacheData;

static Array _data_array = NULL;
static Queue _data_graveyard = NULL;

bool frequency_spectrum_changed(const APE_FrequencySpectrum* left, const APE_FrequencySpectrum* right)
{
    return  left->m_SampleRate      != right->m_SampleRate      ||
            left->m_Frequency       != right->m_Frequency       ||
            left->m_Bandwidth       != right->m_Bandwidth       ||
            left->m_BandwidthGain   != right->m_BandwidthGain   ||
            left->m_ReferenceGain   != right->m_ReferenceGain   ||
            left->m_GainAdjustment  != right->m_GainAdjustment;    
}


void ape_run_filter(APE_EqualizerHandle handle, const APE_FrequencySpectrum* frequncy_sample, const APE_Sample* const in_samples, APE_Sample* out_samples, uint32_t num_samples)
{
    APE_CacheData* data = array_get(_data_array, handle);
    assert(data != NULL && "Invalid handle points to incorrect data.");

    // recalculate our filter coefficients if our spectrum parameters have changed
    if(frequency_spectrum_changed(&data->m_Spectrum, frequncy_sample))
    {
        // Maths based on https://8void.files.wordpress.com/2017/11/orfanidis.pdf
        float gb_calc_0 = pow(10.0, frequncy_sample->m_BandwidthGain / 20.0);
        float g0_calc_0 = pow(10.0, frequncy_sample->m_ReferenceGain / 20.0);
        float g_calc_0  = pow(10.0, frequncy_sample->m_GainAdjustment / 20.0);
        float gb_calc_1 = pow(gb_calc_0, 2.0);
        float g0_calc_1 = pow(g0_calc_0, 2.0);
        float g_calc_1  = pow(g_calc_0, 2.0);
        float fs_half   = frequncy_sample->m_SampleRate / 2.0;

        float beta = tan(frequncy_sample->m_Bandwidth / 2.0 * M_PI / (fs_half)) * 
                    sqrt(fabs(gb_calc_1 - g0_calc_1)) / sqrt(fabs(0.001 + g_calc_1 - gb_calc_1));

        float beta_p = 1.0 + beta;
        float beta_m = 1.0 - beta;
        float f0_cos_x2 = -2.0 * cos(frequncy_sample->m_Frequency * M_PI / fs_half) / beta_p;

        data->m_Spectrum = *frequncy_sample;
        data->m_B0 = (g0_calc_0 + g_calc_0 * beta) / beta_p;
        data->m_B1 =  g0_calc_0 * f0_cos_x2;
        data->m_B2 = (g0_calc_0 - g_calc_0 * beta) / beta_p;
        data->m_A0 = 1;
        data->m_A1 = f0_cos_x2;
        data->m_A2 = beta_m / beta_p;
    }

    // apply the filter with our coefficients
    const APE_Sample* in_0;
    const APE_Sample* in_1;
    const APE_Sample* in_2;
    APE_Sample* out_0;
    APE_Sample* out_1;
    APE_Sample* out_2;

    // run the first 2 with our cached data
    in_0 = &in_samples[0];
    in_1 = &data->m_RawSamples[0];
    in_2 = &data->m_RawSamples[1];
    out_0 = &out_samples[0];
    out_1 = &data->m_ProcessedSamples[0];
    out_2 = &data->m_ProcessedSamples[1];
    *out_0 = ((data->m_B0 * *in_0) + 
                (data->m_B1 * *in_1) + 
                (data->m_B2 * *in_2) - 
                (data->m_A1 * *out_1) - 
                (data->m_A2 * *out_2)) * data->m_A0;

    in_0 = &in_samples[1];
    in_1 = &in_samples[0];;
    in_2 = &data->m_RawSamples[0];
    out_0 = &out_samples[1];
    out_1 = &out_samples[0];
    out_2 = &data->m_ProcessedSamples[0];
    *out_0 = ((data->m_B0 * *in_0) + 
                (data->m_B1 * *in_1) + 
                (data->m_B2 * *in_2) - 
                (data->m_A1 * *out_1) - 
                (data->m_A2 * *out_2)) * data->m_A0;

    // run the rest in loop
    for(uint32_t sample_index = 2; sample_index < num_samples; ++sample_index)
    {
        in_0 = &in_samples[sample_index];
        in_1 = &in_samples[sample_index - 1];
        in_2 = &in_samples[sample_index - 2];
        out_0 = &out_samples[sample_index];
        out_1 = &out_samples[sample_index - 1];
        out_2 = &out_samples[sample_index - 2];

        *out_0 = ((data->m_B0 * *in_0) + 
                  (data->m_B1 * *in_1) + 
                  (data->m_B2 * *in_2) - 
                  (data->m_A1 * *out_1) - 
                  (data->m_A2 * *out_2)) * data->m_A0;
    }

    // cache the end values
    data->m_ProcessedSamples[0] = out_samples[num_samples - 1];
    data->m_ProcessedSamples[1] = out_samples[num_samples - 2];
    data->m_RawSamples[0]   = in_samples[num_samples - 1];
    data->m_RawSamples[1]   = in_samples[num_samples - 2];
}

APE_EqualizerHandle ape_obtain()
{
    if(_data_array == NULL)
    {
        _data_array = array_create(8, false);
    }

    APE_CacheData* target_cache = NULL;
    if(_data_graveyard != NULL && !queue_is_empty(_data_graveyard))
    {
        target_cache = queue_pop_front(_data_graveyard);
        memset(((uint8_t*)target_cache) + sizeof(APE_EqualizerHandle), 0, sizeof(APE_CacheData) -  sizeof(APE_EqualizerHandle));
    }
    else
    {
        target_cache = calloc(1, sizeof(APE_CacheData));
        assert(target_cache != NULL && "Unable to allocate new cache data");

        target_cache->m_Handle = array_size(_data_array);
        assert(array_push_back(_data_array, target_cache) && "Unable to push new data to our storage.");
    }
    return target_cache->m_Handle;    
}

void ape_return(APE_EqualizerHandle handle)
{
    if(_data_graveyard == NULL)
    {
        _data_graveyard = queue_create();
    }

    APE_CacheData* data = array_get(_data_array, handle);
    assert(data != NULL && "Invalid handle points to incorrect data.");
    queue_push_back(_data_graveyard, data);

    // we should be ok to shutdown if we have returned everything we have allocated
    if(queue_size(_data_graveyard) == array_size(_data_array))
    {
        queue_destroy(_data_graveyard);
        for(uint32_t array_index = 0; array_index < array_size(_data_array); ++array_index)
        {
            free(array_get(_data_array, array_index));
        }
        array_destroy(_data_array);
    }
}
