#include "capture.h"
#include "feature_extraction.h"
#include "hashing.h"
#include "template.h"
#include "utils.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_banner(void) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║                                                           ║\n");
    printf("║                      N E U R O L O C K                    ║\n");
    printf("║                                                           ║\n");
    printf("║         Multi-factor Authentication via Brainwaves       ║\n");
    printf("║                                                           ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

void print_usage(const char *program_name) {
    printf("Usage: %s <command> [options]\n\n", program_name);
    printf("Commands:\n");
    printf("  enroll <username>       Enroll a new user\n");
    printf("  auth <username>         Authenticate a user\n");
    printf("  delete <username>       Delete user template\n");
    printf("  list                    List enrolled users\n");
    printf("  test                    Run system test\n");
    printf("  help                    Show this help message\n");
    printf("\n");
    printf("Options:\n");
    printf("  --device <name>         Specify EEG device name/path\n");
    printf("  --task <type>           Mental task type (0-4)\n");
    printf("                          0: Eyes closed rest (default)\n");
    printf("                          1: Eyes open rest\n");
    printf("                          2: Mental arithmetic\n");
    printf("                          3: Motor imagery\n");
    printf("                          4: Visual imagery\n");
    printf("\n");
}

int cmd_enroll(const char *username, const char *device_name, MentalTask task) {
    printf("\n");
    printf("========================================\n");
    printf("         USER ENROLMENT\n");
    printf("========================================\n");
    printf("Username: %s\n", username);
    printf("Enrolment trials: %d\n", NUM_ENROLMENT_TRIALS);
    printf("========================================\n\n");
    
    // Check if user already exists
    if (template_exists(username)) {
        printf("Error: User '%s' already enrolled.\n", username);
        printf("Delete existing template first with: neurolock delete %s\n", username);
        return -1;
    }
    
    // Initialize capture system
    if (capture_init() != 0) {
        log_message(LOG_ERROR, "Failed to initialize capture system");
        return -1;
    }
    
    // Connect to device
    if (capture_connect(device_name) != 0) {
        log_message(LOG_ERROR, "Failed to connect to device");
        capture_cleanup();
        return -1;
    }
    
    // Start streaming
    if (capture_start_streaming() != 0) {
        log_message(LOG_ERROR, "Failed to start streaming");
        capture_cleanup();
        return -1;
    }
    
    // Capture multiple trials
    EEGData **trials = (EEGData**)malloc(NUM_ENROLMENT_TRIALS * sizeof(EEGData*));
    if (!trials) {
        log_message(LOG_ERROR, "Failed to allocate trial array");
        capture_cleanup();
        return -1;
    }
    
    printf("You will perform %d trials. Try to maintain consistency.\n\n", NUM_ENROLMENT_TRIALS);
    
    for (int i = 0; i < NUM_ENROLMENT_TRIALS; i++) {
        printf("=== Trial %d/%d ===\n", i + 1, NUM_ENROLMENT_TRIALS);
        
        trials[i] = eeg_data_alloc(NUM_CHANNELS, SAMPLING_RATE * CAPTURE_DURATION);
        if (!trials[i]) {
            log_message(LOG_ERROR, "Failed to allocate trial data");
            // Cleanup previous trials
            for (int j = 0; j < i; j++) {
                eeg_data_free(trials[j]);
            }
            free(trials);
            capture_cleanup();
            return -1;
        }
        
        if (capture_record(CAPTURE_DURATION, task, trials[i]) != 0) {
            log_message(LOG_ERROR, "Failed to capture trial");
            // Cleanup
            for (int j = 0; j <= i; j++) {
                eeg_data_free(trials[j]);
            }
            free(trials);
            capture_cleanup();
            return -1;
        }
        
        printf("\n");
        
        if (i < NUM_ENROLMENT_TRIALS - 1) {
            printf("Rest for 10 seconds before next trial...\n");
            sleep_ms(10000);
        }
    }
    
    // Create template
    printf("\nCreating template...\n");
    Template *template = template_alloc();
    if (!template) {
        log_message(LOG_ERROR, "Failed to allocate template");
        // Cleanup
        for (int i = 0; i < NUM_ENROLMENT_TRIALS; i++) {
            eeg_data_free(trials[i]);
        }
        free(trials);
        capture_cleanup();
        return -1;
    }
    
    if (template_create(username, (const EEGData**)trials, NUM_ENROLMENT_TRIALS, task, template) != 0) {
        log_message(LOG_ERROR, "Failed to create template");
        template_free(template);
        for (int i = 0; i < NUM_ENROLMENT_TRIALS; i++) {
            eeg_data_free(trials[i]);
        }
        free(trials);
        capture_cleanup();
        return -1;
    }
    
    // Save template
    char filepath[512];
    template_get_filepath(username, filepath, sizeof(filepath));
    
    if (template_save(template, filepath) != 0) {
        log_message(LOG_ERROR, "Failed to save template");
        template_free(template);
        for (int i = 0; i < NUM_ENROLMENT_TRIALS; i++) {
            eeg_data_free(trials[i]);
        }
        free(trials);
        capture_cleanup();
        return -1;
    }
    
    printf("\n");
    printf("========================================\n");
    printf("  ✓ ENROLMENT SUCCESSFUL\n");
    printf("========================================\n");
    printf("Template saved to: %s\n", filepath);
    printf("\n");
    
    // Cleanup
    template_free(template);
    for (int i = 0; i < NUM_ENROLMENT_TRIALS; i++) {
        eeg_data_free(trials[i]);
    }
    free(trials);
    capture_cleanup();
    
    return 0;
}

int cmd_authenticate(const char *username, const char *device_name, MentalTask task) {
    printf("\n");
    printf("========================================\n");
    printf("         USER AUTHENTICATION\n");
    printf("========================================\n");
    printf("Username: %s\n", username);
    printf("========================================\n\n");
    
    // Check if user exists
    if (!template_exists(username)) {
        printf("Error: User '%s' not enrolled.\n", username);
        printf("Enroll first with: neurolock enroll %s\n", username);
        return -1;
    }
    
    // Load template
    Template *template = template_alloc();
    if (!template) {
        log_message(LOG_ERROR, "Failed to allocate template");
        return -1;
    }
    
    char filepath[512];
    template_get_filepath(username, filepath, sizeof(filepath));
    
    if (template_load(filepath, template) != 0) {
        log_message(LOG_ERROR, "Failed to load template");
        template_free(template);
        return -1;
    }
    
    // Initialize capture system
    if (capture_init() != 0) {
        log_message(LOG_ERROR, "Failed to initialize capture system");
        template_free(template);
        return -1;
    }
    
    // Connect to device
    if (capture_connect(device_name) != 0) {
        log_message(LOG_ERROR, "Failed to connect to device");
        template_free(template);
        capture_cleanup();
        return -1;
    }
    
    // Start streaming
    if (capture_start_streaming() != 0) {
        log_message(LOG_ERROR, "Failed to start streaming");
        template_free(template);
        capture_cleanup();
        return -1;
    }
    
    // Capture authentication trial
    EEGData *trial = eeg_data_alloc(NUM_CHANNELS, SAMPLING_RATE * CAPTURE_DURATION);
    if (!trial) {
        log_message(LOG_ERROR, "Failed to allocate trial data");
        template_free(template);
        capture_cleanup();
        return -1;
    }
    
    if (capture_record(CAPTURE_DURATION, template->task_type, trial) != 0) {
        log_message(LOG_ERROR, "Failed to capture trial");
        eeg_data_free(trial);
        template_free(template);
        capture_cleanup();
        return -1;
    }
    
    // Authenticate
    printf("\nAuthenticating...\n");
    AuthResult result;
    
    if (template_authenticate(trial, template, &result) != 0) {
        log_message(LOG_ERROR, "Authentication process failed");
        eeg_data_free(trial);
        template_free(template);
        capture_cleanup();
        return -1;
    }
    
    printf("\n");
    printf("========================================\n");
    if (result.authenticated) {
        printf("  ✓ AUTHENTICATION SUCCESSFUL\n");
        printf("========================================\n");
        printf("Similarity score: %.3f\n", result.similarity_score);
        printf("Threshold: %.3f\n", SIMILARITY_THRESHOLD);
    } else {
        printf("  ✗ AUTHENTICATION FAILED\n");
        printf("========================================\n");
        printf("Similarity score: %.3f\n", result.similarity_score);
        printf("Threshold: %.3f\n", SIMILARITY_THRESHOLD);
        printf("Access denied.\n");
    }
    printf("\n");
    
    // Cleanup
    eeg_data_free(trial);
    template_free(template);
    capture_cleanup();
    
    return result.authenticated ? 0 : -1;
}

int cmd_delete(const char *username) {
    printf("\nDeleting template for user: %s\n", username);
    
    if (!template_exists(username)) {
        printf("Error: User '%s' not enrolled.\n", username);
        return -1;
    }
    
    char confirm[10];
    printf("Are you sure? (yes/no): ");
    if (get_user_input("", confirm, sizeof(confirm))) {
        if (strcmp(confirm, "yes") == 0) {
            if (template_delete(username) == 0) {
                printf("Template deleted successfully.\n");
                return 0;
            } else {
                printf("Failed to delete template.\n");
                return -1;
            }
        } else {
            printf("Deletion cancelled.\n");
            return 0;
        }
    }
    
    return -1;
}

int cmd_test(void) {
    printf("\n");
    printf("========================================\n");
    printf("         SYSTEM TEST\n");
    printf("========================================\n\n");
    
    printf("Testing capture system...\n");
    if (capture_init() == 0) {
        printf("  ✓ Capture initialization: OK\n");
        capture_cleanup();
    } else {
        printf("  ✗ Capture initialization: FAILED\n");
    }
    
    printf("\nTesting feature extraction...\n");
    EEGData *test_data = eeg_data_alloc(NUM_CHANNELS, SAMPLING_RATE * 2);
    if (test_data) {
        // Fill with test data
        for (size_t i = 0; i < test_data->num_channels * test_data->num_samples; i++) {
            test_data->data[i] = ((float)rand() / RAND_MAX - 0.5f) * 100.0f;
        }
        
        FeatureVector *features = feature_vector_alloc(FEATURE_VECTOR_SIZE);
        if (features && extract_features(test_data, features) == 0) {
            printf("  ✓ Feature extraction: OK\n");
            feature_vector_free(features);
        } else {
            printf("  ✗ Feature extraction: FAILED\n");
        }
        
        eeg_data_free(test_data);
    } else {
        printf("  ✗ Data allocation: FAILED\n");
    }
    
    printf("\nTesting hashing...\n");
    uint8_t salt[SALT_LENGTH];
    if (generate_salt(salt, SALT_LENGTH) == 0) {
        printf("  ✓ Salt generation: OK\n");
    } else {
        printf("  ✗ Salt generation: FAILED\n");
    }
    
    printf("\n========================================\n");
    printf("  SYSTEM TEST COMPLETE\n");
    printf("========================================\n\n");
    
    return 0;
}

int main(int argc, char *argv[]) {
    print_banner();
    
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char *command = argv[1];
    const char *device_name = "default_eeg_device"; // Default device
    MentalTask task = TASK_EYES_CLOSED_REST; // Default task
    
    // Parse options
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--device") == 0 && i + 1 < argc) {
            device_name = argv[++i];
        } else if (strcmp(argv[i], "--task") == 0 && i + 1 < argc) {
            task = (MentalTask)atoi(argv[++i]);
        }
    }
    
    // Execute command
    if (strcmp(command, "enroll") == 0) {
        if (argc < 3) {
            printf("Error: Username required\n");
            print_usage(argv[0]);
            return 1;
        }
        return cmd_enroll(argv[2], device_name, task);
        
    } else if (strcmp(command, "auth") == 0 || strcmp(command, "authenticate") == 0) {
        if (argc < 3) {
            printf("Error: Username required\n");
            print_usage(argv[0]);
            return 1;
        }
        return cmd_authenticate(argv[2], device_name, task);
        
    } else if (strcmp(command, "delete") == 0) {
        if (argc < 3) {
            printf("Error: Username required\n");
            print_usage(argv[0]);
            return 1;
        }
        return cmd_delete(argv[2]);
        
    } else if (strcmp(command, "test") == 0) {
        return cmd_test();
        
    } else if (strcmp(command, "help") == 0) {
        print_usage(argv[0]);
        return 0;
        
    } else {
        printf("Error: Unknown command '%s'\n", command);
        print_usage(argv[0]);
        return 1;
    }
    
    return 0;
}
