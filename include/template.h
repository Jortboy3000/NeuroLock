#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <stdint.h>
#include <time.h>
#include "feature_extraction.h"
#include "hashing.h"
#include "config.h"

/* Template Structure */
typedef struct {
    char username[64];              // User identifier
    HashData *hash;                 // Hashed template
    FeatureVector *features;        // Feature vector (for similarity matching)
    MentalTask task_type;           // Associated mental task
    time_t created_at;              // Creation timestamp
    time_t last_used;               // Last authentication timestamp
    uint32_t version;               // Template version
} Template;

/* Authentication Result */
typedef struct {
    int authenticated;              // 1 if authenticated, 0 if not
    float similarity_score;         // Similarity score (0-1)
    time_t timestamp;               // Authentication timestamp
    int attempts;                   // Number of attempts made
} AuthResult;

/* Function Prototypes */

/**
 * Create a new template from multiple EEG trials
 * @param username: User identifier
 * @param trials: Array of EEG data from multiple trials
 * @param num_trials: Number of trials
 * @param task: Mental task type
 * @param output: Output template
 * Returns: 0 on success, negative on error
 */
int template_create(const char *username, const EEGData **trials, size_t num_trials, MentalTask task, Template *output);

/**
 * Save template to disk
 * @param template: Template to save
 * @param filepath: Path to save template file
 * Returns: 0 on success, negative on error
 */
int template_save(const Template *template, const char *filepath);

/**
 * Load template from disk
 * @param filepath: Path to template file
 * @param output: Output template
 * Returns: 0 on success, negative on error
 */
int template_load(const char *filepath, Template *output);

/**
 * Authenticate user against stored template
 * @param trial: EEG data from authentication attempt
 * @param template: Stored template to compare against
 * @param result: Output authentication result
 * Returns: 0 on success, negative on error
 */
int template_authenticate(const EEGData *trial, const Template *template, AuthResult *result);

/**
 * Calculate cosine similarity between two feature vectors
 * @param vec1: First feature vector
 * @param vec2: Second feature vector
 * Returns: Similarity score (0-1), negative on error
 */
float calculate_similarity(const FeatureVector *vec1, const FeatureVector *vec2);

/**
 * Calculate Hamming distance between two hashes
 * @param hash1: First hash
 * @param hash2: Second hash
 * Returns: Hamming distance, negative on error
 */
int calculate_hamming_distance(const HashData *hash1, const HashData *hash2);

/**
 * Update template with new authentication data (adaptive learning)
 * @param template: Template to update
 * @param trial: New EEG trial data
 * Returns: 0 on success, negative on error
 */
int template_update(Template *template, const EEGData *trial);

/**
 * Allocate memory for Template
 * Returns: Pointer to allocated Template, NULL on failure
 */
Template* template_alloc(void);

/**
 * Free Template structure
 * @param template: Pointer to Template to free
 */
void template_free(Template *template);

/**
 * Get default template filepath for user
 * @param username: User identifier
 * @param output: Output buffer for filepath
 * @param size: Size of output buffer
 * Returns: 0 on success, negative on error
 */
int template_get_filepath(const char *username, char *output, size_t size);

/**
 * Check if template exists for user
 * @param username: User identifier
 * Returns: 1 if exists, 0 if not
 */
int template_exists(const char *username);

/**
 * Delete template for user
 * @param username: User identifier
 * Returns: 0 on success, negative on error
 */
int template_delete(const char *username);

#endif /* TEMPLATE_H */
