#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>

/* Logging Levels */
typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARNING = 2,
    LOG_ERROR = 3,
    LOG_CRITICAL = 4
} LogLevel;

/* Function Prototypes */

/**
 * Log a message with specified level
 * @param level: Log level
 * @param format: Printf-style format string
 * @param ...: Variable arguments
 */
void log_message(LogLevel level, const char *format, ...);

/**
 * Get current timestamp in milliseconds
 * Returns: Current timestamp
 */
uint64_t get_timestamp_ms(void);

/**
 * Get current time as formatted string
 * @param buffer: Output buffer
 * @param size: Size of buffer
 * Returns: Pointer to buffer
 */
char* get_time_string(char *buffer, size_t size);

/**
 * Sleep for specified milliseconds
 * @param ms: Milliseconds to sleep
 */
void sleep_ms(uint32_t ms);

/**
 * Create directory if it doesn't exist
 * @param path: Directory path
 * Returns: 0 on success, negative on error
 */
int create_directory(const char *path);

/**
 * Check if file exists
 * @param filepath: Path to file
 * Returns: 1 if exists, 0 if not
 */
int file_exists(const char *filepath);

/**
 * Read entire file into buffer
 * @param filepath: Path to file
 * @param buffer: Output buffer (allocated by function)
 * @param size: Output size of file
 * Returns: 0 on success, negative on error
 */
int read_file(const char *filepath, uint8_t **buffer, size_t *size);

/**
 * Write buffer to file
 * @param filepath: Path to file
 * @param buffer: Input buffer
 * @param size: Size of buffer
 * Returns: 0 on success, negative on error
 */
int write_file(const char *filepath, const uint8_t *buffer, size_t size);

/**
 * Display progress bar
 * @param current: Current progress value
 * @param total: Total value
 * @param message: Optional message to display
 */
void display_progress(size_t current, size_t total, const char *message);

/**
 * Get user input string
 * @param prompt: Prompt to display
 * @param buffer: Output buffer
 * @param size: Size of buffer
 * Returns: Pointer to buffer
 */
char* get_user_input(const char *prompt, char *buffer, size_t size);

/**
 * Display countdown timer
 * @param seconds: Number of seconds to count down
 * @param message: Message to display
 */
void countdown_timer(int seconds, const char *message);

/**
 * Calculate mean of float array
 * @param data: Input array
 * @param size: Size of array
 * Returns: Mean value
 */
float calculate_mean(const float *data, size_t size);

/**
 * Calculate standard deviation of float array
 * @param data: Input array
 * @param size: Size of array
 * Returns: Standard deviation
 */
float calculate_std_dev(const float *data, size_t size);

/**
 * Calculate dot product of two vectors
 * @param vec1: First vector
 * @param vec2: Second vector
 * @param size: Size of vectors
 * Returns: Dot product
 */
float dot_product(const float *vec1, const float *vec2, size_t size);

/**
 * Calculate magnitude (L2 norm) of vector
 * @param vec: Input vector
 * @param size: Size of vector
 * Returns: Magnitude
 */
float vector_magnitude(const float *vec, size_t size);

/**
 * Print error message and exit
 * @param message: Error message
 */
void fatal_error(const char *message);

#endif /* UTILS_H */
