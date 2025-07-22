# NeuroLock
Brainwave-Based Multi-Factor Authentication

NeuroLock is a research prototype and proof of concept for a biometric security system that uses EEG (electroencephalography) signals as a second factor multi-fact auth (MFA).

Rather than relying on traditional biometric data like fingerprints, facial scans or iris scans, NeuroLock captures and processes unique brainwaive patterns to validate user identity.
This project will explore whether thought-based biometrics can provide a non-invaisive, secure and continuous authetication method for high sensitity enviroments.

Key objectives:
- Investigate the feasibility of EEG signals as a stable, unique biometric identifier.
- Build a C-based signal processing pipeline to extract and match EEG signatures in real-time.
- Validate Auth against both known user patterms and spoofing attempts.
- Asses practicality and limitations in real-world use.

Performance Goals:

- Metric	Target
- Authentication Time	< 3 seconds.
- False Acceptance Rate	< 5%.
- False Rejection Rate	< 10%.
- Signal Stability	70%+ repeatability across days.

Security Model:

Threat Model: Includes spoofing (replay attacks), noise injection, forced authentication, and profile leakage.

Mitigation:

Challenge-response EEG tasks
AES-256 encrypted EEG vectors
Threshold-based vector similarity
Local processing only (no cloud transmission)
