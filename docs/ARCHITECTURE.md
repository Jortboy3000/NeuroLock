# NeuroLock Architecture

## Overview

NeuroLock is a modular biometric authentication system that uses EEG (electroencephalography) signals as a form of "brainprint" for multi-factor authentication. The system is designed to be low-level, efficient, and portable, written entirely in C.

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        Main Application                      │
│                          (main.c)                            │
└────────────┬────────────────────────────────────┬───────────┘
             │                                    │
             ▼                                    ▼
┌─────────────────────────┐        ┌─────────────────────────┐
│   Enrolment Pipeline    │        │ Authentication Pipeline │
└────────┬────────────────┘        └────────┬────────────────┘
         │                                    │
         ▼                                    ▼
┌─────────────────────────────────────────────────────────────┐
│                     EEG Capture Module                       │
│                        (capture.c)                           │
│  • Device connection management                              │
│  • Real-time data streaming                                  │
│  • Mental task guidance                                      │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│               Feature Extraction Module                      │
│                  (feature_extraction.c)                      │
│  • Signal preprocessing (filtering, normalization)           │
│  • Artifact removal (EOG, EMG)                               │
│  • Frequency band power extraction                           │
│  • FFT/Wavelet transforms                                    │
└────────────────────────┬────────────────────────────────────┘
                         │
         ┌───────────────┴───────────────┐
         ▼                               ▼
┌──────────────────────┐      ┌──────────────────────┐
│   Hashing Module     │      │   Template Module    │
│     (hashing.c)      │      │    (template.c)      │
│  • Salted hashing    │◄────►│  • Template creation │
│  • SHA-256/BLAKE3    │      │  • Similarity match  │
│  • Secure wiping     │      │  • Storage/retrieval │
└──────────────────────┘      └──────────────────────┘
         │                               │
         └───────────────┬───────────────┘
                         ▼
┌─────────────────────────────────────────────────────────────┐
│                    Storage (File System)                     │
│                    templates/*.nlt                           │
└─────────────────────────────────────────────────────────────┘
```

## Module Details

### 1. EEG Capture Module (`capture.c`)

**Responsibilities:**
- Initialize and manage EEG device connections
- Stream real-time EEG data from multiple channels
- Display mental task instructions to users
- Buffer and organize raw signal data

**Key Functions:**
- `capture_init()` - Initialize capture system
- `capture_connect()` - Connect to EEG device
- `capture_record()` - Record EEG data for specified duration
- `capture_display_task_instructions()` - Guide user through mental tasks

**Data Structures:**
```c
typedef struct {
    float *data;              // Raw EEG samples
    size_t num_channels;      // Number of EEG channels
    size_t num_samples;       // Samples per channel
    float sampling_rate;      // Hz
    uint64_t timestamp;       // Capture time
    MentalTask task_type;     // Mental task performed
} EEGData;
```

### 2. Feature Extraction Module (`feature_extraction.c`)

**Responsibilities:**
- Preprocess raw EEG signals
- Remove artifacts and noise
- Extract discriminative features from signals
- Normalize and prepare data for matching

**Pipeline:**
1. **Filtering:**
   - Bandpass filter (0.5-50 Hz)
   - Notch filter (50/60 Hz power line noise)
   
2. **Artifact Removal:**
   - EOG (eye movement) rejection
   - EMG (muscle) artifact removal
   
3. **Normalization:**
   - Zero-mean, unit variance per channel
   
4. **Feature Extraction:**
   - FFT-based power spectral density
   - Frequency band powers (Delta, Theta, Alpha, Beta, Gamma)
   - Optional: Wavelet coefficients

**Data Structures:**
```c
typedef struct {
    float *features;          // Feature values
    size_t size;              // Number of features
    MentalTask task_type;     // Associated task
    uint64_t timestamp;       // Extraction time
} FeatureVector;
```

### 3. Hashing Module (`hashing.c`)

**Responsibilities:**
- Generate cryptographically secure random salts
- Hash feature vectors with salts
- Provide non-reversible template storage
- Prevent replay attacks

**Security Features:**
- **Salted Hashing:** Each template uses unique salt
- **One-way Functions:** SHA-256 (BLAKE3 optional)
- **Secure Memory Wiping:** Clear sensitive data after use
- **Constant-time Comparison:** Prevent timing attacks

**Data Structures:**
```c
typedef struct {
    uint8_t *hash;            // Hash bytes
    size_t hash_size;         // Hash length
    uint8_t *salt;            // Salt bytes
    size_t salt_size;         // Salt length
} HashData;
```

### 4. Template Module (`template.c`)

**Responsibilities:**
- Create user templates from multiple trials
- Store and retrieve templates
- Perform authentication matching
- Manage template lifecycle

**Enrolment Process:**
1. Capture N trials (default: 3)
2. Extract features from each trial
3. Average feature vectors for robustness
4. Generate unique salt
5. Hash averaged features
6. Save template to disk

**Authentication Process:**
1. Load stored template
2. Capture new trial
3. Extract features
4. Calculate cosine similarity with template
5. Compare against threshold (default: 0.85)
6. Return authentication result

**Data Structures:**
```c
typedef struct {
    char username[64];        // User ID
    HashData *hash;           // Hashed template
    FeatureVector *features;  // Feature vector (for matching)
    MentalTask task_type;     // Task used
    time_t created_at;        // Creation time
    time_t last_used;         // Last auth time
    uint32_t version;         // Template version
} Template;
```

### 5. Utility Module (`utils.c`)

**Responsibilities:**
- Logging and debugging
- File I/O operations
- Math utilities (mean, std dev, dot product)
- User interface helpers (progress bars, timers)
- Cross-platform compatibility

## Data Flow

### Enrolment Flow

```
User Input → Mental Task Instructions
     ↓
EEG Device → Capture Raw Signals (Trial 1)
     ↓
Preprocessing → Filtering, Artifact Removal
     ↓
Feature Extraction → Frequency Band Powers
     ↓
[Repeat for Trials 2, 3]
     ↓
Average Features → Robust Template
     ↓
Generate Salt → Random 32 bytes
     ↓
Hash Features + Salt → SHA-256
     ↓
Save to Disk → templates/username.nlt
```

### Authentication Flow

```
User Input → Load Stored Template
     ↓
Mental Task Instructions (Same as Enrolment)
     ↓
EEG Device → Capture Raw Signals
     ↓
Preprocessing → Filtering, Artifact Removal
     ↓
Feature Extraction → Frequency Band Powers
     ↓
Similarity Calculation → Cosine Similarity
     ↓
Threshold Comparison → >= 0.85?
     ↓
[Yes] → Authentication Success
[No]  → Authentication Failed
```

## Security Model

### Threat Model

**Protected Against:**
- **Eavesdropping:** Raw EEG never stored, only hashed templates
- **Replay Attacks:** Salt ensures different hash each time
- **Template Theft:** Hash is non-reversible
- **Timing Attacks:** Constant-time hash comparison

**NOT Protected Against (yet):**
- **Coercion:** User forced to authenticate
- **Presentation Attacks:** Pre-recorded EEG signals
- **Advanced Signal Injection:** Requires additional liveness detection

### Privacy Considerations

1. **No Raw EEG Storage:** Only feature vectors and hashes stored
2. **Local Processing:** All computation done on-device
3. **User Control:** Users can delete templates at any time
4. **Transparent Hashing:** Salt and hash algorithm documented

## Performance Characteristics

### Time Complexity

- **Enrolment:** O(N × M) where N = trials, M = samples
- **Authentication:** O(M) where M = samples
- **Feature Extraction:** O(M × log M) due to FFT
- **Template Matching:** O(F) where F = feature vector size

### Space Complexity

- **Raw EEG Buffer:** ~640 KB (8 channels × 5 sec × 256 Hz × 4 bytes)
- **Feature Vector:** ~160 bytes (40 features × 4 bytes)
- **Template File:** ~500 bytes (features + hash + metadata)

### Latency

- **Enrolment:** ~60 seconds (3 trials × 5 sec + processing)
- **Authentication:** ~10 seconds (1 trial × 5 sec + processing)

## Extensibility

### Adding New Mental Tasks

1. Add task to `MentalTask` enum in `config.h`
2. Update `capture_display_task_instructions()` in `capture.c`
3. No other changes needed (system is task-agnostic)

### Adding New Feature Types

1. Implement extraction function in `feature_extraction.c`
2. Update `FEATURE_VECTOR_SIZE` in `config.h`
3. Modify `extract_features()` to include new features

### Adding New Hash Algorithms

1. Implement hash function in `hashing.c`
2. Update `hash_features()` to select algorithm
3. Update `USE_SHA256` flag in `config.h`

## Platform Support

### Currently Supported
- Linux (tested)
- macOS (should work, needs testing)
- Windows (partial support, needs testing)

### Device Support
- **Planned:** OpenBCI, Muse 2, Emotiv EPOC
- **Current:** Simulated device (for development)

## Future Enhancements

1. **Real Device Integration:** OpenBCI/Muse drivers
2. **Machine Learning:** Neural network feature extraction
3. **Liveness Detection:** Detect pre-recorded signals
4. **Multi-Modal:** Combine with other biometrics
5. **Adaptive Templates:** Update over time
6. **Secure Enclave:** Hardware-backed key storage
7. **Zero-Knowledge Proofs:** Authenticate without revealing template

## References

See `docs/RESEARCH.md` for academic references and theoretical foundations.
