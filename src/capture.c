#include "capture.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global device state */
static DeviceStatus device_status = DEVICE_DISCONNECTED;
static char device_name[256] = {0};

/**
 * Initialize EEG capture device
 */
int capture_init(void) {
    log_message(LOG_INFO, "Initializing EEG capture system...");
    
    // Initialize device communication buffers
    // TODO: Initialize device-specific drivers (OpenBCI, Muse, etc.)
    
    device_status = DEVICE_DISCONNECTED;
    log_message(LOG_INFO, "EEG capture system initialized");
    return 0;
}

/**
 * Connect to EEG device
 */
int capture_connect(const char *dev_name) {
    if (!dev_name) {
        log_message(LOG_ERROR, "Device name is NULL");
        return -1;
    }
    
    log_message(LOG_INFO, "Connecting to device: %s", dev_name);
    
    // TODO: Implement actual device connection
    // For now, simulate connection
    strncpy(device_name, dev_name, sizeof(device_name) - 1);
    device_status = DEVICE_CONNECTED;
    
    log_message(LOG_INFO, "Connected to device: %s", dev_name);
    return 0;
}

/**
 * Check device connection status
 */
DeviceStatus capture_get_status(void) {
    return device_status;
}

/**
 * Start streaming EEG data
 */
int capture_start_streaming(void) {
    if (device_status != DEVICE_CONNECTED) {
        log_message(LOG_ERROR, "Device not connected");
        return -1;
    }
    
    log_message(LOG_INFO, "Starting EEG stream...");
    
    // TODO: Send start streaming command to device
    device_status = DEVICE_STREAMING;
    
    log_message(LOG_INFO, "EEG streaming started");
    return 0;
}

/**
 * Stop streaming EEG data
 */
int capture_stop_streaming(void) {
    if (device_status != DEVICE_STREAMING) {
        log_message(LOG_WARNING, "Device not streaming");
        return -1;
    }
    
    log_message(LOG_INFO, "Stopping EEG stream...");
    
    // TODO: Send stop streaming command to device
    device_status = DEVICE_CONNECTED;
    
    log_message(LOG_INFO, "EEG streaming stopped");
    return 0;
}

/**
 * Display instructions for mental task
 */
void capture_display_task_instructions(MentalTask task) {
    printf("\n");
    printf("========================================\n");
    printf("          MENTAL TASK INSTRUCTIONS     \n");
    printf("========================================\n\n");
    
    switch (task) {
        case TASK_EYES_CLOSED_REST:
            printf("Task: Eyes Closed Resting State\n\n");
            printf("Instructions:\n");
            printf("1. Sit in a comfortable position\n");
            printf("2. Close your eyes\n");
            printf("3. Relax and clear your mind\n");
            printf("4. Breathe normally\n");
            printf("5. Try to stay still for %d seconds\n", CAPTURE_DURATION);
            break;
            
        case TASK_EYES_OPEN_REST:
            printf("Task: Eyes Open Resting State\n\n");
            printf("Instructions:\n");
            printf("1. Sit in a comfortable position\n");
            printf("2. Keep your eyes open\n");
            printf("3. Focus on a point in front of you\n");
            printf("4. Relax and breathe normally\n");
            printf("5. Try to stay still for %d seconds\n", CAPTURE_DURATION);
            break;
            
        case TASK_MENTAL_ARITHMETIC:
            printf("Task: Mental Arithmetic\n\n");
            printf("Instructions:\n");
            printf("1. Sit in a comfortable position\n");
            printf("2. You will be given a math problem\n");
            printf("3. Solve it in your head (don't speak)\n");
            printf("4. Example: Count backwards from 100 by 7\n");
            printf("5. Continue for %d seconds\n", CAPTURE_DURATION);
            break;
            
        case TASK_MOTOR_IMAGERY:
            printf("Task: Motor Imagery\n\n");
            printf("Instructions:\n");
            printf("1. Sit in a comfortable position\n");
            printf("2. Imagine moving your right hand\n");
            printf("3. Don't actually move it - just imagine\n");
            printf("4. Visualize the movement clearly\n");
            printf("5. Continue for %d seconds\n", CAPTURE_DURATION);
            break;
            
        case TASK_VISUAL_IMAGERY:
            printf("Task: Visual Imagery\n\n");
            printf("Instructions:\n");
            printf("1. Sit in a comfortable position\n");
            printf("2. Close your eyes\n");
            printf("3. Imagine a peaceful scene (beach, forest)\n");
            printf("4. Visualize it vividly with details\n");
            printf("5. Continue for %d seconds\n", CAPTURE_DURATION);
            break;
            
        default:
            printf("Unknown task type\n");
            break;
    }
    
    printf("\n========================================\n\n");
}

/**
 * Capture EEG data for a specified duration
 */
int capture_record(float duration, MentalTask task, EEGData *output) {
    if (!output) {
        log_message(LOG_ERROR, "Output pointer is NULL");
        return -1;
    }
    
    if (device_status != DEVICE_STREAMING) {
        log_message(LOG_ERROR, "Device not streaming");
        return -1;
    }
    
    // Display task instructions
    capture_display_task_instructions(task);
    
    // Give user time to prepare
    countdown_timer(3, "Starting capture in");
    
    // Calculate buffer size
    size_t num_samples = (size_t)(duration * SAMPLING_RATE);
    size_t num_channels = NUM_CHANNELS;
    
    // Allocate EEG data structure
    EEGData *data = eeg_data_alloc(num_channels, num_samples);
    if (!data) {
        log_message(LOG_ERROR, "Failed to allocate EEG data");
        return -1;
    }
    
    data->sampling_rate = SAMPLING_RATE;
    data->task_type = task;
    data->timestamp = get_timestamp_ms();
    
    log_message(LOG_INFO, "Recording EEG data for %.1f seconds...", duration);
    
    // TODO: Actual data capture from device
    // For now, simulate with random data
    for (size_t i = 0; i < num_channels * num_samples; i++) {
        // Simulate EEG data (normally distributed around 0, with typical EEG amplitude)
        data->data[i] = ((float)rand() / RAND_MAX - 0.5f) * 100.0f;
    }
    
    // Display progress
    for (int i = 0; i <= (int)duration; i++) {
        display_progress(i, (int)duration, "Capturing EEG data");
        sleep_ms(1000);
    }
    
    printf("\n");
    log_message(LOG_INFO, "EEG data capture complete");
    
    // Copy to output
    memcpy(output, data, sizeof(EEGData));
    output->data = data->data;
    free(data); // Free structure but not data buffer (transferred to output)
    
    return 0;
}

/**
 * Disconnect from EEG device
 */
int capture_disconnect(void) {
    if (device_status == DEVICE_STREAMING) {
        capture_stop_streaming();
    }
    
    log_message(LOG_INFO, "Disconnecting from device...");
    
    // TODO: Actual device disconnection
    device_status = DEVICE_DISCONNECTED;
    memset(device_name, 0, sizeof(device_name));
    
    log_message(LOG_INFO, "Device disconnected");
    return 0;
}

/**
 * Clean up and free resources
 */
void capture_cleanup(void) {
    if (device_status != DEVICE_DISCONNECTED) {
        capture_disconnect();
    }
    
    log_message(LOG_INFO, "Capture system cleanup complete");
}

/**
 * Allocate memory for EEGData structure
 */
EEGData* eeg_data_alloc(size_t num_channels, size_t num_samples) {
    EEGData *data = (EEGData*)malloc(sizeof(EEGData));
    if (!data) {
        log_message(LOG_ERROR, "Failed to allocate EEGData structure");
        return NULL;
    }
    
    data->data = (float*)calloc(num_channels * num_samples, sizeof(float));
    if (!data->data) {
        log_message(LOG_ERROR, "Failed to allocate EEG data buffer");
        free(data);
        return NULL;
    }
    
    data->num_channels = num_channels;
    data->num_samples = num_samples;
    data->sampling_rate = SAMPLING_RATE;
    data->timestamp = 0;
    data->task_type = TASK_EYES_CLOSED_REST;
    
    return data;
}

/**
 * Free EEGData structure
 */
void eeg_data_free(EEGData *data) {
    if (data) {
        if (data->data) {
            secure_wipe(data->data, data->num_channels * data->num_samples * sizeof(float));
            free(data->data);
        }
        free(data);
    }
}
