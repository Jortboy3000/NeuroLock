#include "template.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <openssl/sha.h>
#include <openssl/evp.h>

/**
 * Create a new template from multiple EEG trials
 */
int template_create(const char *username, const EEGData **trials, size_t num_trials, MentalTask task, Template *output) {
    if (!username || !trials || !output || num_trials == 0) {
        log_message(LOG_ERROR, "Invalid input for template creation");
        return -1;
    }
    
    log_message(LOG_INFO, "Creating template for user: %s", username);
    
    // Extract features from all trials
    FeatureVector **feature_vectors = (FeatureVector**)malloc(num_trials * sizeof(FeatureVector*));
    if (!feature_vectors) {
        log_message(LOG_ERROR, "Failed to allocate feature vector array");
        return -1;
    }
    
    for (size_t i = 0; i < num_trials; i++) {
        feature_vectors[i] = feature_vector_alloc(FEATURE_VECTOR_SIZE);
        if (!feature_vectors[i]) {
            log_message(LOG_ERROR, "Failed to allocate feature vector");
            // Cleanup
            for (size_t j = 0; j < i; j++) {
                feature_vector_free(feature_vectors[j]);
            }
            free(feature_vectors);
            return -1;
        }
        
        if (extract_features(trials[i], feature_vectors[i]) != 0) {
            log_message(LOG_ERROR, "Failed to extract features from trial %zu", i);
            // Cleanup
            for (size_t j = 0; j <= i; j++) {
                feature_vector_free(feature_vectors[j]);
            }
            free(feature_vectors);
            return -1;
        }
    }
    
    // Average feature vectors
    output->features = feature_vector_alloc(FEATURE_VECTOR_SIZE);
    if (!output->features) {
        log_message(LOG_ERROR, "Failed to allocate averaged feature vector");
        // Cleanup
        for (size_t i = 0; i < num_trials; i++) {
            feature_vector_free(feature_vectors[i]);
        }
        free(feature_vectors);
        return -1;
    }
    
    if (average_feature_vectors((const FeatureVector**)feature_vectors, num_trials, output->features) != 0) {
        log_message(LOG_ERROR, "Failed to average feature vectors");
        // Cleanup
        for (size_t i = 0; i < num_trials; i++) {
            feature_vector_free(feature_vectors[i]);
        }
        free(feature_vectors);
        feature_vector_free(output->features);
        return -1;
    }
    
    // Generate salt
    output->hash = hash_data_alloc(HASH_OUTPUT_SIZE, SALT_LENGTH);
    if (!output->hash) {
        log_message(LOG_ERROR, "Failed to allocate hash data");
        // Cleanup
        for (size_t i = 0; i < num_trials; i++) {
            feature_vector_free(feature_vectors[i]);
        }
        free(feature_vectors);
        feature_vector_free(output->features);
        return -1;
    }
    
    if (generate_salt(output->hash->salt, SALT_LENGTH) != 0) {
        log_message(LOG_ERROR, "Failed to generate salt");
        // Cleanup
        for (size_t i = 0; i < num_trials; i++) {
            feature_vector_free(feature_vectors[i]);
        }
        free(feature_vectors);
        feature_vector_free(output->features);
        hash_data_free(output->hash);
        return -1;
    }
    
    // Hash the averaged features
    if (hash_features(output->features, output->hash->salt, SALT_LENGTH, output->hash) != 0) {
        log_message(LOG_ERROR, "Failed to hash features");
        // Cleanup
        for (size_t i = 0; i < num_trials; i++) {
            feature_vector_free(feature_vectors[i]);
        }
        free(feature_vectors);
        feature_vector_free(output->features);
        hash_data_free(output->hash);
        return -1;
    }
    
    // Set template metadata
    strncpy(output->username, username, sizeof(output->username) - 1);
    output->username[sizeof(output->username) - 1] = '\0';
    output->task_type = task;
    output->created_at = time(NULL);
    output->last_used = output->created_at;
    output->version = 1;
    
    // Cleanup
    for (size_t i = 0; i < num_trials; i++) {
        feature_vector_free(feature_vectors[i]);
    }
    free(feature_vectors);
    
    log_message(LOG_INFO, "Template created successfully");
    return 0;
}

/**
 * Save template to disk
 */
int template_save(const Template *template, const char *filepath) {
    if (!template || !filepath) {
        log_message(LOG_ERROR, "Invalid input for template save");
        return -1;
    }
    
    log_message(LOG_INFO, "Saving template to: %s", filepath);
    
    // Ensure template directory exists
    if (create_directory(TEMPLATE_DIR) != 0) {
        log_message(LOG_ERROR, "Failed to create template directory");
        return -1;
    }
    
    FILE *fp = fopen(filepath, "wb");
    if (!fp) {
        log_message(LOG_ERROR, "Failed to open file for writing: %s", filepath);
        return -1;
    }
    
    // Write template metadata
    fwrite(&template->version, sizeof(uint32_t), 1, fp);
    fwrite(template->username, sizeof(char), 64, fp);
    fwrite(&template->task_type, sizeof(MentalTask), 1, fp);
    fwrite(&template->created_at, sizeof(time_t), 1, fp);
    fwrite(&template->last_used, sizeof(time_t), 1, fp);
    
    // Write feature vector
    fwrite(&template->features->size, sizeof(size_t), 1, fp);
    fwrite(template->features->features, sizeof(float), template->features->size, fp);
    
    // Write hash data
    fwrite(&template->hash->hash_size, sizeof(size_t), 1, fp);
    fwrite(template->hash->hash, sizeof(uint8_t), template->hash->hash_size, fp);
    fwrite(&template->hash->salt_size, sizeof(size_t), 1, fp);
    fwrite(template->hash->salt, sizeof(uint8_t), template->hash->salt_size, fp);
    
    fclose(fp);
    
    log_message(LOG_INFO, "Template saved successfully");
    return 0;
}

/**
 * Load template from disk
 */
int template_load(const char *filepath, Template *output) {
    if (!filepath || !output) {
        log_message(LOG_ERROR, "Invalid input for template load");
        return -1;
    }
    
    log_message(LOG_INFO, "Loading template from: %s", filepath);
    
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        log_message(LOG_ERROR, "Failed to open file for reading: %s", filepath);
        return -1;
    }
    
    // Read template metadata
    fread(&output->version, sizeof(uint32_t), 1, fp);
    fread(output->username, sizeof(char), 64, fp);
    fread(&output->task_type, sizeof(MentalTask), 1, fp);
    fread(&output->created_at, sizeof(time_t), 1, fp);
    fread(&output->last_used, sizeof(time_t), 1, fp);
    
    // Read feature vector
    size_t feature_size;
    fread(&feature_size, sizeof(size_t), 1, fp);
    output->features = feature_vector_alloc(feature_size);
    if (!output->features) {
        fclose(fp);
        return -1;
    }
    fread(output->features->features, sizeof(float), feature_size, fp);
    
    // Read hash data
    size_t hash_size, salt_size;
    fread(&hash_size, sizeof(size_t), 1, fp);
    fread(&salt_size, sizeof(size_t), 1, fp);
    
    // Allocate hash data before reading salt_size again
    output->hash = hash_data_alloc(hash_size, salt_size);
    if (!output->hash) {
        feature_vector_free(output->features);
        fclose(fp);
        return -1;
    }
    
    // Seek back to read hash and salt
    fseek(fp, -(long)(sizeof(size_t)), SEEK_CUR);
    fread(&output->hash->salt_size, sizeof(size_t), 1, fp);
    
    fseek(fp, -(long)(sizeof(size_t) * 2 + hash_size), SEEK_CUR);
    fread(output->hash->hash, sizeof(uint8_t), hash_size, fp);
    fseek(fp, sizeof(size_t), SEEK_CUR);
    fread(output->hash->salt, sizeof(uint8_t), salt_size, fp);
    
    fclose(fp);
    
    log_message(LOG_INFO, "Template loaded successfully");
    return 0;
}

/**
 * Calculate cosine similarity between two feature vectors
 */
float calculate_similarity(const FeatureVector *vec1, const FeatureVector *vec2) {
    if (!vec1 || !vec2 || !vec1->features || !vec2->features) {
        log_message(LOG_ERROR, "Invalid feature vectors for similarity calculation");
        return -1.0f;
    }
    
    if (vec1->size != vec2->size) {
        log_message(LOG_ERROR, "Feature vectors have different sizes");
        return -1.0f;
    }
    
    float dot = dot_product(vec1->features, vec2->features, vec1->size);
    float mag1 = vector_magnitude(vec1->features, vec1->size);
    float mag2 = vector_magnitude(vec2->features, vec2->size);
    
    if (mag1 < 1e-6f || mag2 < 1e-6f) {
        log_message(LOG_ERROR, "Zero magnitude vector");
        return -1.0f;
    }
    
    float similarity = dot / (mag1 * mag2);
    
    // Clamp to [0, 1]
    if (similarity < 0.0f) similarity = 0.0f;
    if (similarity > 1.0f) similarity = 1.0f;
    
    return similarity;
}

/**
 * Calculate Hamming distance between two hashes
 */
int calculate_hamming_distance(const HashData *hash1, const HashData *hash2) {
    if (!hash1 || !hash2) {
        log_message(LOG_ERROR, "Invalid hashes for Hamming distance");
        return -1;
    }
    
    if (hash1->hash_size != hash2->hash_size) {
        log_message(LOG_ERROR, "Hashes have different sizes");
        return -1;
    }
    
    int distance = 0;
    for (size_t i = 0; i < hash1->hash_size; i++) {
        uint8_t xor_result = hash1->hash[i] ^ hash2->hash[i];
        // Count set bits
        while (xor_result) {
            distance += xor_result & 1;
            xor_result >>= 1;
        }
    }
    
    return distance;
}

/**
 * Authenticate user against stored template
 */
int template_authenticate(const EEGData *trial, const Template *template, AuthResult *result) {
    if (!trial || !template || !result) {
        log_message(LOG_ERROR, "Invalid input for authentication");
        return -1;
    }
    
    log_message(LOG_INFO, "Authenticating against template for user: %s", template->username);
    
    // Extract features from trial
    FeatureVector *trial_features = feature_vector_alloc(FEATURE_VECTOR_SIZE);
    if (!trial_features) {
        log_message(LOG_ERROR, "Failed to allocate trial feature vector");
        return -1;
    }
    
    if (extract_features(trial, trial_features) != 0) {
        log_message(LOG_ERROR, "Failed to extract features from trial");
        feature_vector_free(trial_features);
        return -1;
    }
    
    // Calculate similarity
    float similarity = calculate_similarity(trial_features, template->features);
    
    result->similarity_score = similarity;
    result->timestamp = time(NULL);
    result->attempts = 1;
    
    // Check if similarity exceeds threshold
    if (similarity >= SIMILARITY_THRESHOLD) {
        result->authenticated = 1;
        log_message(LOG_INFO, "Authentication SUCCESSFUL (similarity: %.3f)", similarity);
    } else {
        result->authenticated = 0;
        log_message(LOG_WARNING, "Authentication FAILED (similarity: %.3f < %.3f)", 
                   similarity, SIMILARITY_THRESHOLD);
    }
    
    feature_vector_free(trial_features);
    return 0;
}

/**
 * Update template with new authentication data
 */
int template_update(Template *template, const EEGData *trial) {
    // TODO: Implement adaptive learning
    // This would update the template with new successful authentication data
    log_message(LOG_INFO, "Template update not yet implemented");
    return 0;
}

/**
 * Allocate memory for Template
 */
Template* template_alloc(void) {
    Template *template = (Template*)calloc(1, sizeof(Template));
    if (!template) {
        log_message(LOG_ERROR, "Failed to allocate Template structure");
        return NULL;
    }
    
    return template;
}

/**
 * Free Template structure
 */
void template_free(Template *template) {
    if (template) {
        if (template->features) {
            feature_vector_free(template->features);
        }
        if (template->hash) {
            hash_data_free(template->hash);
        }
        secure_wipe(template, sizeof(Template));
        free(template);
    }
}

/**
 * Get default template filepath for user
 */
int template_get_filepath(const char *username, char *output, size_t size) {
    if (!username || !output) {
        log_message(LOG_ERROR, "Invalid input for filepath generation");
        return -1;
    }
    
    snprintf(output, size, "%s/%s%s", TEMPLATE_DIR, username, TEMPLATE_EXTENSION);
    return 0;
}

/**
 * Check if template exists for user
 */
int template_exists(const char *username) {
    char filepath[512];
    template_get_filepath(username, filepath, sizeof(filepath));
    return file_exists(filepath);
}

/**
 * Delete template for user
 */
int template_delete(const char *username) {
    char filepath[512];
    template_get_filepath(username, filepath, sizeof(filepath));
    
    if (remove(filepath) != 0) {
        log_message(LOG_ERROR, "Failed to delete template: %s", filepath);
        return -1;
    }
    
    log_message(LOG_INFO, "Template deleted: %s", filepath);
    return 0;
}
