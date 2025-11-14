#include "utils.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
#else
    #include <unistd.h>
#endif

/**
 * Log a message with specified level
 */
void log_message(LogLevel level, const char *format, ...) {
    #if DEBUG_MODE
    const char *level_str[] = {"DEBUG", "INFO", "WARN", "ERROR", "CRITICAL"};
    const char *level_color[] = {"\033[36m", "\033[32m", "\033[33m", "\033[31m", "\033[35m"};
    const char *reset_color = "\033[0m";
    
    // Skip debug messages if verbose logging is off
    if (level == LOG_DEBUG && !VERBOSE_LOGGING) {
        return;
    }
    
    // Get timestamp
    char time_buf[32];
    get_time_string(time_buf, sizeof(time_buf));
    
    // Print log header
    fprintf(stderr, "%s[%s] [%s]%s ", level_color[level], time_buf, level_str[level], reset_color);
    
    // Print message
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    
    fprintf(stderr, "\n");
    #endif
}

/**
 * Get current timestamp in milliseconds
 */
uint64_t get_timestamp_ms(void) {
    #ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    uint64_t time = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    return time / 10000; // Convert to milliseconds
    #else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
    #endif
}

/**
 * Get current time as formatted string
 */
char* get_time_string(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
    return buffer;
}

/**
 * Sleep for specified milliseconds
 */
void sleep_ms(uint32_t ms) {
    #ifdef _WIN32
    Sleep(ms);
    #else
    usleep(ms * 1000);
    #endif
}

/**
 * Create directory if it doesn't exist
 */
int create_directory(const char *path) {
    struct stat st = {0};
    
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0700) != 0) {
            log_message(LOG_ERROR, "Failed to create directory: %s", path);
            return -1;
        }
        log_message(LOG_INFO, "Created directory: %s", path);
    }
    
    return 0;
}

/**
 * Check if file exists
 */
int file_exists(const char *filepath) {
    struct stat st;
    return (stat(filepath, &st) == 0);
}

/**
 * Read entire file into buffer
 */
int read_file(const char *filepath, uint8_t **buffer, size_t *size) {
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        log_message(LOG_ERROR, "Failed to open file: %s", filepath);
        return -1;
    }
    
    // Get file size
    fseek(fp, 0, SEEK_END);
    *size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    // Allocate buffer
    *buffer = (uint8_t*)malloc(*size);
    if (!*buffer) {
        log_message(LOG_ERROR, "Failed to allocate buffer for file");
        fclose(fp);
        return -1;
    }
    
    // Read file
    size_t read_size = fread(*buffer, 1, *size, fp);
    fclose(fp);
    
    if (read_size != *size) {
        log_message(LOG_ERROR, "Failed to read complete file");
        free(*buffer);
        return -1;
    }
    
    return 0;
}

/**
 * Write buffer to file
 */
int write_file(const char *filepath, const uint8_t *buffer, size_t size) {
    FILE *fp = fopen(filepath, "wb");
    if (!fp) {
        log_message(LOG_ERROR, "Failed to open file for writing: %s", filepath);
        return -1;
    }
    
    size_t written = fwrite(buffer, 1, size, fp);
    fclose(fp);
    
    if (written != size) {
        log_message(LOG_ERROR, "Failed to write complete file");
        return -1;
    }
    
    return 0;
}

/**
 * Display progress bar
 */
void display_progress(size_t current, size_t total, const char *message) {
    const int bar_width = 40;
    float progress = (float)current / (float)total;
    int pos = (int)(bar_width * progress);
    
    printf("\r%s: [", message ? message : "Progress");
    for (int i = 0; i < bar_width; i++) {
        if (i < pos) printf("=");
        else if (i == pos) printf(">");
        else printf(" ");
    }
    printf("] %3d%%", (int)(progress * 100));
    fflush(stdout);
}

/**
 * Get user input string
 */
char* get_user_input(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    fflush(stdout);
    
    if (fgets(buffer, size, stdin) == NULL) {
        return NULL;
    }
    
    // Remove newline
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len-1] == '\n') {
        buffer[len-1] = '\0';
    }
    
    return buffer;
}

/**
 * Display countdown timer
 */
void countdown_timer(int seconds, const char *message) {
    for (int i = seconds; i > 0; i--) {
        printf("\r%s: %d seconds... ", message ? message : "Wait", i);
        fflush(stdout);
        sleep_ms(1000);
    }
    printf("\rStarting now!              \n");
    fflush(stdout);
}

/**
 * Calculate mean of float array
 */
float calculate_mean(const float *data, size_t size) {
    if (size == 0) return 0.0f;
    
    float sum = 0.0f;
    for (size_t i = 0; i < size; i++) {
        sum += data[i];
    }
    
    return sum / (float)size;
}

/**
 * Calculate standard deviation of float array
 */
float calculate_std_dev(const float *data, size_t size) {
    if (size == 0) return 0.0f;
    
    float mean = calculate_mean(data, size);
    float sum_sq_diff = 0.0f;
    
    for (size_t i = 0; i < size; i++) {
        float diff = data[i] - mean;
        sum_sq_diff += diff * diff;
    }
    
    return sqrtf(sum_sq_diff / (float)size);
}

/**
 * Calculate dot product of two vectors
 */
float dot_product(const float *vec1, const float *vec2, size_t size) {
    float result = 0.0f;
    
    for (size_t i = 0; i < size; i++) {
        result += vec1[i] * vec2[i];
    }
    
    return result;
}

/**
 * Calculate magnitude (L2 norm) of vector
 */
float vector_magnitude(const float *vec, size_t size) {
    float sum_sq = 0.0f;
    
    for (size_t i = 0; i < size; i++) {
        sum_sq += vec[i] * vec[i];
    }
    
    return sqrtf(sum_sq);
}

/**
 * Print error message and exit
 */
void fatal_error(const char *message) {
    log_message(LOG_CRITICAL, "FATAL ERROR: %s", message);
    fprintf(stderr, "\nFATAL ERROR: %s\n", message);
    exit(EXIT_FAILURE);
}
