#ifndef CAPTURE_H
#define CAPTURE_H

#include <stdint.h>
#include <stddef.h>
#include "config.h"

/* EEG Data Structure */
typedef struct {
    float *data;                    // Raw EEG data [channels * samples]
    size_t num_channels;            // Number of channels
    size_t num_samples;             // Number of samples per channel
    float sampling_rate;            // Sampling rate in Hz
    uint64_t timestamp;             // Capture timestamp
    MentalTask task_type;           // Type of mental task performed
} EEGData;

/* Device Connection Status */
typedef enum {
    DEVICE_DISCONNECTED = 0,
    DEVICE_CONNECTED = 1,
    DEVICE_STREAMING = 2,
    DEVICE_ERROR = -1
} DeviceStatus;

/* Function Prototypes */

/**
 * Initialize EEG capture device
 * Returns: 0 on success, negative on error
 */
int capture_init(void);

/**
 * Connect to EEG device
 * @param device_name: Name or path of the EEG device
 * Returns: 0 on success, negative on error
 */
int capture_connect(const char *device_name);

/**
 * Check device connection status
 * Returns: DeviceStatus enum value
 */
DeviceStatus capture_get_status(void);

/**
 * Start streaming EEG data
 * Returns: 0 on success, negative on error
 */
int capture_start_streaming(void);

/**
 * Stop streaming EEG data
 * Returns: 0 on success, negative on error
 */
int capture_stop_streaming(void);

/**
 * Capture EEG data for a specified duration
 * @param duration: Duration in seconds
 * @param task: Mental task to perform
 * @param output: Pointer to store captured data
 * Returns: 0 on success, negative on error
 */
int capture_record(float duration, MentalTask task, EEGData *output);

/**
 * Disconnect from EEG device
 * Returns: 0 on success, negative on error
 */
int capture_disconnect(void);

/**
 * Clean up and free resources
 */
void capture_cleanup(void);

/**
 * Allocate memory for EEGData structure
 * @param num_channels: Number of EEG channels
 * @param num_samples: Number of samples per channel
 * Returns: Pointer to allocated EEGData, NULL on failure
 */
EEGData* eeg_data_alloc(size_t num_channels, size_t num_samples);

/**
 * Free EEGData structure
 * @param data: Pointer to EEGData to free
 */
void eeg_data_free(EEGData *data);

/**
 * Display instructions for mental task
 * @param task: Mental task type
 */
void capture_display_task_instructions(MentalTask task);

#endif /* CAPTURE_H */
