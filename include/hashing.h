#ifndef HASHING_H
#define HASHING_H

#include <stdint.h>
#include <stddef.h>
#include "feature_extraction.h"
#include "config.h"

/* Hash Structure */
typedef struct {
    uint8_t *hash;                  // Hash bytes
    size_t hash_size;               // Size of hash in bytes
    uint8_t *salt;                  // Salt bytes
    size_t salt_size;               // Size of salt in bytes
} HashData;

/* Function Prototypes */

/**
 * Generate a cryptographically secure random salt
 * @param salt: Buffer to store salt
 * @param salt_size: Size of salt to generate
 * Returns: 0 on success, negative on error
 */
int generate_salt(uint8_t *salt, size_t salt_size);

/**
 * Hash feature vector with salt using SHA-256
 * @param features: Input feature vector
 * @param salt: Salt bytes
 * @param salt_size: Size of salt
 * @param output: Output hash data
 * Returns: 0 on success, negative on error
 */
int hash_features_sha256(const FeatureVector *features, const uint8_t *salt, size_t salt_size, HashData *output);

/**
 * Hash feature vector with salt using BLAKE3
 * @param features: Input feature vector
 * @param salt: Salt bytes
 * @param salt_size: Size of salt
 * @param output: Output hash data
 * Returns: 0 on success, negative on error
 */
int hash_features_blake3(const FeatureVector *features, const uint8_t *salt, size_t salt_size, HashData *output);

/**
 * Hash feature vector with salt (uses configured algorithm)
 * @param features: Input feature vector
 * @param salt: Salt bytes
 * @param salt_size: Size of salt
 * @param output: Output hash data
 * Returns: 0 on success, negative on error
 */
int hash_features(const FeatureVector *features, const uint8_t *salt, size_t salt_size, HashData *output);

/**
 * Compare two hashes for equality
 * @param hash1: First hash
 * @param hash2: Second hash
 * Returns: 1 if equal, 0 if not equal
 */
int hash_compare(const HashData *hash1, const HashData *hash2);

/**
 * Allocate memory for HashData
 * @param hash_size: Size of hash
 * @param salt_size: Size of salt
 * Returns: Pointer to allocated HashData, NULL on failure
 */
HashData* hash_data_alloc(size_t hash_size, size_t salt_size);

/**
 * Free HashData structure
 * @param data: Pointer to HashData to free
 */
void hash_data_free(HashData *data);

/**
 * Convert hash to hexadecimal string
 * @param hash: Input hash data
 * @param output: Output string buffer (must be at least hash_size*2 + 1 bytes)
 * Returns: 0 on success, negative on error
 */
int hash_to_hex(const HashData *hash, char *output);

/**
 * Securely wipe memory containing sensitive data
 * @param ptr: Pointer to memory to wipe
 * @param size: Size of memory to wipe
 */
void secure_wipe(void *ptr, size_t size);

#endif /* HASHING_H */
