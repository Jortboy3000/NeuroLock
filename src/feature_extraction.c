#include "feature_extraction.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * Apply bandpass filter to EEG data
 */
int filter_bandpass(EEGData *data, float low_freq, float high_freq) {
    if (!data || !data->data) {
        log_message(LOG_ERROR, "Invalid EEG data for filtering");
        return -1;
    }
    
    log_message(LOG_INFO, "Applying bandpass filter (%.1f-%.1f Hz)", low_freq, high_freq);
    
    // TODO: Implement proper IIR or FIR bandpass filter
    // For now, this is a placeholder
    // A real implementation would use Butterworth, Chebyshev, or similar filter
    
    return 0;
}

/**
 * Apply notch filter to remove power line interference
 */
int filter_notch(EEGData *data, float notch_freq) {
    if (!data || !data->data) {
        log_message(LOG_ERROR, "Invalid EEG data for filtering");
        return -1;
    }
    
    log_message(LOG_INFO, "Applying notch filter at %.1f Hz", notch_freq);
    
    // TODO: Implement proper notch filter (IIR notch)
    // For now, this is a placeholder
    
    return 0;
}

/**
 * Remove eye blink artifacts using EOG rejection
 */
int remove_eye_artifacts(EEGData *data) {
    if (!data || !data->data) {
        log_message(LOG_ERROR, "Invalid EEG data for artifact removal");
        return -1;
    }
    
    log_message(LOG_INFO, "Removing eye blink artifacts");
    
    // TODO: Implement EOG artifact rejection
    // Methods: threshold-based rejection, ICA, or regression
    
    return 0;
}

/**
 * Normalize EEG signal (zero mean, unit variance)
 */
int normalize_signal(EEGData *data) {
    if (!data || !data->data) {
        log_message(LOG_ERROR, "Invalid EEG data for normalization");
        return -1;
    }
    
    log_message(LOG_DEBUG, "Normalizing EEG signal");
    
    size_t total_samples = data->num_channels * data->num_samples;
    
    // Calculate mean and std dev for each channel
    for (size_t ch = 0; ch < data->num_channels; ch++) {
        float *channel_data = data->data + (ch * data->num_samples);
        
        float mean = calculate_mean(channel_data, data->num_samples);
        float std_dev = calculate_std_dev(channel_data, data->num_samples);
        
        // Avoid division by zero
        if (std_dev < 1e-6f) {
            std_dev = 1.0f;
        }
        
        // Normalize: (x - mean) / std_dev
        for (size_t i = 0; i < data->num_samples; i++) {
            channel_data[i] = (channel_data[i] - mean) / std_dev;
        }
    }
    
    log_message(LOG_DEBUG, "Signal normalization complete");
    return 0;
}

/**
 * Perform Fast Fourier Transform (simplified version)
 */
int compute_fft(const float *input, float *output, size_t size) {
    if (!input || !output) {
        log_message(LOG_ERROR, "Invalid FFT input/output");
        return -1;
    }
    
    // TODO: Implement proper FFT (use FFTW library or similar)
    // For now, use a simple DFT for demonstration
    // This is computationally expensive and should be replaced with FFT
    
    for (size_t k = 0; k < size / 2; k++) {
        float real = 0.0f;
        float imag = 0.0f;
        
        for (size_t n = 0; n < size; n++) {
            float angle = 2.0f * M_PI * k * n / size;
            real += input[n] * cosf(angle);
            imag -= input[n] * sinf(angle);
        }
        
        // Magnitude
        output[k] = sqrtf(real * real + imag * imag);
    }
    
    return 0;
}

/**
 * Extract frequency band power features
 */
int extract_band_power(const EEGData *data, FeatureVector *output) {
    if (!data || !data->data || !output) {
        log_message(LOG_ERROR, "Invalid input for band power extraction");
        return -1;
    }
    
    log_message(LOG_INFO, "Extracting frequency band power features");
    
    // Allocate FFT output buffer
    size_t fft_size = WINDOW_SIZE / 2;
    float *fft_output = (float*)malloc(fft_size * sizeof(float));
    if (!fft_output) {
        log_message(LOG_ERROR, "Failed to allocate FFT output buffer");
        return -1;
    }
    
    // Extract features for each channel
    size_t feature_idx = 0;
    
    for (size_t ch = 0; ch < data->num_channels; ch++) {
        float *channel_data = data->data + (ch * data->num_samples);
        
        // Compute FFT on a window
        if (data->num_samples >= WINDOW_SIZE) {
            compute_fft(channel_data, fft_output, WINDOW_SIZE);
            
            // Calculate power in each frequency band
            float freq_resolution = data->sampling_rate / (float)WINDOW_SIZE;
            
            // Delta band
            float delta_power = 0.0f;
            for (size_t i = (size_t)(DELTA_LOW / freq_resolution); 
                 i < (size_t)(DELTA_HIGH / freq_resolution) && i < fft_size; i++) {
                delta_power += fft_output[i] * fft_output[i];
            }
            output->features[feature_idx++] = delta_power;
            
            // Theta band
            float theta_power = 0.0f;
            for (size_t i = (size_t)(THETA_LOW / freq_resolution); 
                 i < (size_t)(THETA_HIGH / freq_resolution) && i < fft_size; i++) {
                theta_power += fft_output[i] * fft_output[i];
            }
            output->features[feature_idx++] = theta_power;
            
            // Alpha band
            float alpha_power = 0.0f;
            for (size_t i = (size_t)(ALPHA_LOW / freq_resolution); 
                 i < (size_t)(ALPHA_HIGH / freq_resolution) && i < fft_size; i++) {
                alpha_power += fft_output[i] * fft_output[i];
            }
            output->features[feature_idx++] = alpha_power;
            
            // Beta band
            float beta_power = 0.0f;
            for (size_t i = (size_t)(BETA_LOW / freq_resolution); 
                 i < (size_t)(BETA_HIGH / freq_resolution) && i < fft_size; i++) {
                beta_power += fft_output[i] * fft_output[i];
            }
            output->features[feature_idx++] = beta_power;
            
            // Gamma band
            float gamma_power = 0.0f;
            for (size_t i = (size_t)(GAMMA_LOW / freq_resolution); 
                 i < (size_t)(GAMMA_HIGH / freq_resolution) && i < fft_size; i++) {
                gamma_power += fft_output[i] * fft_output[i];
            }
            output->features[feature_idx++] = gamma_power;
        }
    }
    
    free(fft_output);
    
    log_message(LOG_INFO, "Extracted %zu band power features", feature_idx);
    return 0;
}

/**
 * Extract power spectral density features
 */
int extract_psd_features(const EEGData *data, FeatureVector *output) {
    // For now, PSD features are the same as band power
    return extract_band_power(data, output);
}

/**
 * Extract wavelet transform features
 */
int extract_wavelet_features(const EEGData *data, FeatureVector *output) {
    if (!data || !data->data || !output) {
        log_message(LOG_ERROR, "Invalid input for wavelet feature extraction");
        return -1;
    }
    
    log_message(LOG_INFO, "Extracting wavelet transform features");
    
    // TODO: Implement discrete wavelet transform (DWT)
    // Use Daubechies or Morlet wavelets
    // For now, this is a placeholder
    
    log_message(LOG_WARNING, "Wavelet feature extraction not yet implemented");
    return -1;
}

/**
 * Complete feature extraction pipeline
 */
int extract_features(const EEGData *data, FeatureVector *output) {
    if (!data || !data->data || !output) {
        log_message(LOG_ERROR, "Invalid input for feature extraction");
        return -1;
    }
    
    log_message(LOG_INFO, "Starting feature extraction pipeline");
    
    // Create working copy of data for filtering
    EEGData *filtered_data = eeg_data_alloc(data->num_channels, data->num_samples);
    if (!filtered_data) {
        log_message(LOG_ERROR, "Failed to allocate filtered data");
        return -1;
    }
    
    memcpy(filtered_data->data, data->data, 
           data->num_channels * data->num_samples * sizeof(float));
    filtered_data->num_channels = data->num_channels;
    filtered_data->num_samples = data->num_samples;
    filtered_data->sampling_rate = data->sampling_rate;
    
    // Apply preprocessing
    filter_bandpass(filtered_data, HIGHPASS_CUTOFF, LOWPASS_CUTOFF);
    filter_notch(filtered_data, NOTCH_FREQ);
    remove_eye_artifacts(filtered_data);
    normalize_signal(filtered_data);
    
    // Extract features
    int result = extract_band_power(filtered_data, output);
    
    output->task_type = data->task_type;
    output->timestamp = get_timestamp_ms();
    
    // Cleanup
    eeg_data_free(filtered_data);
    
    if (result == 0) {
        log_message(LOG_INFO, "Feature extraction complete");
    }
    
    return result;
}

/**
 * Allocate memory for FeatureVector
 */
FeatureVector* feature_vector_alloc(size_t size) {
    FeatureVector *vector = (FeatureVector*)malloc(sizeof(FeatureVector));
    if (!vector) {
        log_message(LOG_ERROR, "Failed to allocate FeatureVector structure");
        return NULL;
    }
    
    vector->features = (float*)calloc(size, sizeof(float));
    if (!vector->features) {
        log_message(LOG_ERROR, "Failed to allocate feature array");
        free(vector);
        return NULL;
    }
    
    vector->size = size;
    vector->timestamp = 0;
    vector->task_type = TASK_EYES_CLOSED_REST;
    
    return vector;
}

/**
 * Free FeatureVector structure
 */
void feature_vector_free(FeatureVector *vector) {
    if (vector) {
        if (vector->features) {
            secure_wipe(vector->features, vector->size * sizeof(float));
            free(vector->features);
        }
        free(vector);
    }
}

/**
 * Average multiple feature vectors (for enrolment)
 */
int average_feature_vectors(const FeatureVector **vectors, size_t num_vectors, FeatureVector *output) {
    if (!vectors || !output || num_vectors == 0) {
        log_message(LOG_ERROR, "Invalid input for vector averaging");
        return -1;
    }
    
    size_t size = vectors[0]->size;
    
    // Verify all vectors have same size
    for (size_t i = 1; i < num_vectors; i++) {
        if (vectors[i]->size != size) {
            log_message(LOG_ERROR, "Feature vectors have different sizes");
            return -1;
        }
    }
    
    // Calculate average
    for (size_t j = 0; j < size; j++) {
        float sum = 0.0f;
        for (size_t i = 0; i < num_vectors; i++) {
            sum += vectors[i]->features[j];
        }
        output->features[j] = sum / (float)num_vectors;
    }
    
    output->size = size;
    output->task_type = vectors[0]->task_type;
    output->timestamp = get_timestamp_ms();
    
    log_message(LOG_INFO, "Averaged %zu feature vectors", num_vectors);
    return 0;
}
