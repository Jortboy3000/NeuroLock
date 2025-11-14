#include "hashing.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <wincrypt.h>
#else
    #include <fcntl.h>
    #include <unistd.h>
#endif

// OpenSSL includes for SHA-256

/**
 * Generate a cryptographically secure random salt
 */
int generate_salt(uint8_t *salt, size_t salt_size) {
    if (!salt || salt_size == 0) {
        log_message(LOG_ERROR, "Invalid salt buffer");
        return -1;
    }
    
    log_message(LOG_DEBUG, "Generating %zu-byte random salt", salt_size);
    
    #ifdef _WIN32
    // Use Windows Crypto API
    HCRYPTPROV hCryptProv;
    if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        log_message(LOG_ERROR, "Failed to acquire crypto context");
        return -1;
    }
    
    if (!CryptGenRandom(hCryptProv, salt_size, salt)) {
        log_message(LOG_ERROR, "Failed to generate random salt");
        CryptReleaseContext(hCryptProv, 0);
        return -1;
    }
    
    CryptReleaseContext(hCryptProv, 0);
    #else
    // Use /dev/urandom on Unix-like systems
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        log_message(LOG_ERROR, "Failed to open /dev/urandom");
        return -1;
    }
    
    ssize_t result = read(fd, salt, salt_size);
    close(fd);
    
    if (result != (ssize_t)salt_size) {
        log_message(LOG_ERROR, "Failed to read random data");
        return -1;
    }
    #endif
    
    log_message(LOG_DEBUG, "Salt generated successfully");
    return 0;
}

/**
 * Hash feature vector with salt using SHA-256
 */
int hash_features_sha256(const FeatureVector *features, const uint8_t *salt, size_t salt_size, HashData *output) {
    if (!features || !features->features || !salt || !output) {
        log_message(LOG_ERROR, "Invalid input for SHA-256 hashing");
        return -1;
    }
    
    log_message(LOG_DEBUG, "Hashing features with SHA-256");
    
    // Create SHA-256 context
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) {
        log_message(LOG_ERROR, "Failed to create SHA-256 context");
        return -1;
    }
    
    if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1) {
        log_message(LOG_ERROR, "Failed to initialize SHA-256");
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    
    // Hash feature vector
    if (EVP_DigestUpdate(ctx, features->features, features->size * sizeof(float)) != 1) {
        log_message(LOG_ERROR, "Failed to hash features");
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    
    // Hash salt
    if (EVP_DigestUpdate(ctx, salt, salt_size) != 1) {
        log_message(LOG_ERROR, "Failed to hash salt");
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    
    // Finalize hash
    unsigned int hash_len;
    if (EVP_DigestFinal_ex(ctx, output->hash, &hash_len) != 1) {
        log_message(LOG_ERROR, "Failed to finalize hash");
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    
    EVP_MD_CTX_free(ctx);
    
    output->hash_size = hash_len;
    memcpy(output->salt, salt, salt_size);
    output->salt_size = salt_size;
    
    log_message(LOG_DEBUG, "SHA-256 hash complete (%u bytes)", hash_len);
    return 0;
}

/**
 * Hash feature vector with salt using BLAKE3
 */
int hash_features_blake3(const FeatureVector *features, const uint8_t *salt, size_t salt_size, HashData *output) {
    // TODO: Implement BLAKE3 hashing
    // Requires BLAKE3 library
    log_message(LOG_WARNING, "BLAKE3 hashing not yet implemented");
    return -1;
}

/**
 * Hash feature vector with salt (uses configured algorithm)
 */
int hash_features(const FeatureVector *features, const uint8_t *salt, size_t salt_size, HashData *output) {
    #if USE_SHA256
    return hash_features_sha256(features, salt, salt_size, output);
    #else
    return hash_features_blake3(features, salt, salt_size, output);
    #endif
}

/**
 * Compare two hashes for equality
 */
int hash_compare(const HashData *hash1, const HashData *hash2) {
    if (!hash1 || !hash2) {
        log_message(LOG_ERROR, "Invalid hash for comparison");
        return 0;
    }
    
    if (hash1->hash_size != hash2->hash_size) {
        return 0;
    }
    
    // Constant-time comparison to prevent timing attacks
    int result = 0;
    for (size_t i = 0; i < hash1->hash_size; i++) {
        result |= hash1->hash[i] ^ hash2->hash[i];
    }
    
    return (result == 0) ? 1 : 0;
}

/**
 * Allocate memory for HashData
 */
HashData* hash_data_alloc(size_t hash_size, size_t salt_size) {
    HashData *data = (HashData*)malloc(sizeof(HashData));
    if (!data) {
        log_message(LOG_ERROR, "Failed to allocate HashData structure");
        return NULL;
    }
    
    data->hash = (uint8_t*)calloc(hash_size, sizeof(uint8_t));
    if (!data->hash) {
        log_message(LOG_ERROR, "Failed to allocate hash buffer");
        free(data);
        return NULL;
    }
    
    data->salt = (uint8_t*)calloc(salt_size, sizeof(uint8_t));
    if (!data->salt) {
        log_message(LOG_ERROR, "Failed to allocate salt buffer");
        free(data->hash);
        free(data);
        return NULL;
    }
    
    data->hash_size = hash_size;
    data->salt_size = salt_size;
    
    return data;
}

/**
 * Free HashData structure
 */
void hash_data_free(HashData *data) {
    if (data) {
        if (data->hash) {
            secure_wipe(data->hash, data->hash_size);
            free(data->hash);
        }
        if (data->salt) {
            secure_wipe(data->salt, data->salt_size);
            free(data->salt);
        }
        free(data);
    }
}

/**
 * Convert hash to hexadecimal string
 */
int hash_to_hex(const HashData *hash, char *output) {
    if (!hash || !hash->hash || !output) {
        log_message(LOG_ERROR, "Invalid input for hex conversion");
        return -1;
    }
    
    for (size_t i = 0; i < hash->hash_size; i++) {
        sprintf(output + (i * 2), "%02x", hash->hash[i]);
    }
    output[hash->hash_size * 2] = '\0';
    
    return 0;
}

/**
 * Securely wipe memory containing sensitive data
 */
void secure_wipe(void *ptr, size_t size) {
    if (!ptr || size == 0) {
        return;
    }
    
    // Use volatile to prevent compiler optimization
    volatile uint8_t *p = (volatile uint8_t *)ptr;
    while (size--) {
        *p++ = 0;
    }
    
    #ifdef _WIN32
    SecureZeroMemory(ptr, size);
    #else
    // Additional explicit_bzero on systems that support it
    #ifdef __GLIBC__
    explicit_bzero(ptr, size);
    #endif
    #endif
}
