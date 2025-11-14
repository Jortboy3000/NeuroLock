#ifndef CONFIG_H
#define CONFIG_H

/* NeuroLock Configuration Constants */

/* EEG Capture Settings */
#define SAMPLING_RATE 256           // Hz
#define NUM_CHANNELS 8              // Number of EEG channels
#define CAPTURE_DURATION 5          // seconds
#define BUFFER_SIZE (SAMPLING_RATE * CAPTURE_DURATION * NUM_CHANNELS)

/* Feature Extraction Settings */
#define NUM_FREQUENCY_BANDS 5       // Delta, Theta, Alpha, Beta, Gamma
#define FEATURE_VECTOR_SIZE 40      // NUM_CHANNELS * NUM_FREQUENCY_BANDS
#define WINDOW_SIZE 256             // FFT window size
#define OVERLAP 128                 // Window overlap

/* Frequency Band Definitions (Hz) */
#define DELTA_LOW 0.5
#define DELTA_HIGH 4.0
#define THETA_LOW 4.0
#define THETA_HIGH 8.0
#define ALPHA_LOW 8.0
#define ALPHA_HIGH 13.0
#define BETA_LOW 13.0
#define BETA_HIGH 30.0
#define GAMMA_LOW 30.0
#define GAMMA_HIGH 100.0

/* Filtering Settings */
#define HIGHPASS_CUTOFF 0.5         // Hz
#define LOWPASS_CUTOFF 50.0         // Hz
#define NOTCH_FREQ 50.0             // Hz (or 60 for US)

/* Template Settings */
#define NUM_ENROLMENT_TRIALS 3      // Number of trials to average
#define SIMILARITY_THRESHOLD 0.85   // Cosine similarity threshold (0-1)
#define SALT_LENGTH 32              // bytes

/* Hashing Settings */
#define USE_SHA256 1                // Use SHA-256 (set to 0 for BLAKE3)
#define HASH_OUTPUT_SIZE 32         // bytes (SHA-256 output)

/* Authentication Settings */
#define MAX_AUTH_ATTEMPTS 3
#define AUTH_TIMEOUT 30             // seconds

/* Storage Paths */
#define TEMPLATE_DIR "./templates"
#define TEMPLATE_EXTENSION ".nlt"   // NeuroLock Template

/* Mental Task Types */
typedef enum {
    TASK_EYES_CLOSED_REST = 0,
    TASK_EYES_OPEN_REST = 1,
    TASK_MENTAL_ARITHMETIC = 2,
    TASK_MOTOR_IMAGERY = 3,
    TASK_VISUAL_IMAGERY = 4
} MentalTask;

/* Debug Settings */
#define DEBUG_MODE 1
#define VERBOSE_LOGGING 0

#endif /* CONFIG_H */
