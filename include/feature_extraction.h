#ifndef FEATURE_EXTRACTION_H
#define FEATURE_EXTRACTION_H

#include "capture.h"
#include "config.h"

/* Feature Vector Structure */
typedef struct {
    float *features;                // Feature values
    size_t size;                    // Number of features
    MentalTask task_type;           // Associated mental task
    uint64_t timestamp;             // Extraction timestamp
} FeatureVector;

/* Filter Types */
typedef enum {
    FILTER_HIGHPASS,
    FILTER_LOWPASS,
    FILTER_BANDPASS,
    FILTER_NOTCH
} FilterType;

/* Function Prototypes */

/**
 * Apply bandpass filter to EEG data
 * @param data: Input EEG data
 * @param low_freq: Low cutoff frequency (Hz)
 * @param high_freq: High cutoff frequency (Hz)
 * Returns: 0 on success, negative on error
 */
int filter_bandpass(EEGData *data, float low_freq, float high_freq);

/**
 * Apply notch filter to remove power line interference
 * @param data: Input EEG data
 * @param notch_freq: Frequency to notch out (50 or 60 Hz)
 * Returns: 0 on success, negative on error
 */
int filter_notch(EEGData *data, float notch_freq);

/**
 * Remove eye blink artifacts using EOG rejection
 * @param data: Input EEG data
 * Returns: 0 on success, negative on error
 */
int remove_eye_artifacts(EEGData *data);

/**
 * Normalize EEG signal (zero mean, unit variance)
 * @param data: Input EEG data
 * Returns: 0 on success, negative on error
 */
int normalize_signal(EEGData *data);

/**
 * Extract power spectral density features
 * @param data: Input EEG data
 * @param output: Output feature vector
 * Returns: 0 on success, negative on error
 */
int extract_psd_features(const EEGData *data, FeatureVector *output);

/**
 * Extract frequency band power features
 * @param data: Input EEG data
 * @param output: Output feature vector
 * Returns: 0 on success, negative on error
 */
int extract_band_power(const EEGData *data, FeatureVector *output);

/**
 * Perform Fast Fourier Transform
 * @param input: Input time-domain signal
 * @param output: Output frequency-domain signal
 * @param size: Size of input/output arrays
 * Returns: 0 on success, negative on error
 */
int compute_fft(const float *input, float *output, size_t size);

/**
 * Extract wavelet transform features
 * @param data: Input EEG data
 * @param output: Output feature vector
 * Returns: 0 on success, negative on error
 */
int extract_wavelet_features(const EEGData *data, FeatureVector *output);

/**
 * Complete feature extraction pipeline
 * @param data: Input EEG data
 * @param output: Output feature vector
 * Returns: 0 on success, negative on error
 */
int extract_features(const EEGData *data, FeatureVector *output);

/**
 * Allocate memory for FeatureVector
 * @param size: Number of features
 * Returns: Pointer to allocated FeatureVector, NULL on failure
 */
FeatureVector* feature_vector_alloc(size_t size);

/**
 * Free FeatureVector structure
 * @param vector: Pointer to FeatureVector to free
 */
void feature_vector_free(FeatureVector *vector);

/**
 * Average multiple feature vectors (for enrolment)
 * @param vectors: Array of feature vectors
 * @param num_vectors: Number of vectors to average
 * @param output: Output averaged feature vector
 * Returns: 0 on success, negative on error
 */
int average_feature_vectors(const FeatureVector **vectors, size_t num_vectors, FeatureVector *output);

#endif /* FEATURE_EXTRACTION_H */
