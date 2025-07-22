# NeuroLock

NeuroLock
Multi-factor authentication that uses your brainwaves.

What is NeuroLock?
NeuroLock is a prototype MFA system that adds a second layer of security — your brain. Instead of face scans or fingerprints, it uses EEG signals recorded while you do a specific mental task (like relaxing with your eyes closed, or solving a quick arithmetic problem in your head). That "brainprint" is matched against a template stored during enrolment. If it lines up, you're in.

It’s written in C for performance, control, and to stay low-level enough to eventually run on constrained embedded devices.

The idea is simple: your mind becomes your password.

Why build this?
Traditional biometrics like fingerprints and facial recognition are convenient but fixed — they don’t change, and they can be stolen. EEG signals, on the other hand, are dynamic, hard to fake, and tied to internal mental states. That makes them a promising biometric for high-security or high-trust environments like:

Brain–computer interface (BCI) labs

VR/AR headsets with biometric logins

Military-grade access control

Experimental medtech authentication

How it works (simplified)
You perform a short mental task (e.g., relax and close your eyes for 5 seconds).

NeuroLock captures the EEG signal and extracts key features.

A random salt is generated and concatenated to the vector.

The combined result is hashed using SHA-256 (or BLAKE3, depending on config).

That hash becomes your authentication template.

During login, the same process is repeated. Even if the brainprint is the same, the output hash changes with the salt — so it’s secure and not re-usable.

c
Copy
Edit
EEG_vector = [0.32, 0.14, 0.78, ...];
salt = "X7c$1zAq9p";
combined = EEG_vector || salt;
hash = SHA256(combined);
This approach gives you a non-reversible, privacy-safe token that can’t be linked back to the original EEG data.

Challenges & Questions (and how we're handling them)
Is EEG even unique enough?
Yes — studies show that EEG patterns during controlled tasks (like eyes-closed alpha waves or imagined movement) can be person-specific with good accuracy. For example:

Palaniappan (2008) showed ~96% classification accuracy using mental tasks like visualisation or arithmetic.

Ashby et al. (2011) showed that resting-state EEG can differentiate users reliably over time.

That said, it’s not perfect out of the box. NeuroLock leans on task-specific EEG to reduce noise, and we build templates using multi-trial averaging to smooth things out.

What about variability — fatigue, stress, dodgy electrodes?
EEG is definitely sensitive to internal states, and even tiny shifts in electrode placement can throw off signals. Here's what we do:

Use repeatable mental tasks (e.g., eyes closed, breathing) to guide the user to a stable state

Apply filtering to remove eye blinks, muscle movement, and baseline noise (e.g., EOG rejection, bandpass filters)

Use multiple trials and average them to build a more robust profile

Normalise per session to account for signal drift

Sources:

Palaniappan, 2008 – EEG biometrics using mental tasks

Chuang et al., 2013 – User authentication based on ERP

What if the brainprint gets stolen?
You can’t “reset” your brain like a password — true — but you can change the task or use a new salt. Since the hash depends on the EEG vector + salt, even the same task gives a totally different output each time.

And we never store raw EEG. Only hashed templates — think of it like a fingerprint that's been abstracted and encrypted.

Is this legal or private enough?
In Australia, EEG counts as biometric and health data under the Privacy Act 1988, so it’s classed as sensitive. You can't just hoard it. That’s why:

No raw EEG is ever stored

We hash all templates with one-way functions and rotate salts

Future versions will include transparent “zero-knowledge” protocols to let users verify what's being used without giving away data

Performance?
Still early. We’re experimenting with:

Feature extraction via wavelet transforms and FFTs

Matching using cosine similarity or Hamming distance between template hashes

Noise resilience testing under stress, caffeine, eye strain etc.

Roadmap
✅ Basic template creation
✅ Salted hashing
✅ Signal cleaning pipeline
🛠 Feature selection tuning
🛠 Enrolment interface
🛠 FAR/FRR benchmarking
🔒 Secure storage module
⚡️ Real-time auth demo (Raspberry Pi-compatible)

How to run
Plug in a compatible EEG headset (OpenBCI, Muse 2, or dry-electrode dev boards)

Run the capture.c module

Follow onscreen instructions to record mental tasks

The system will generate and hash your template

(Instructions will get tighter as the project matures.)

⚠️ Disclaimers
NeuroLock is in active development. It’s a research prototype — not production-ready, not medically certified, and definitely not guaranteed to secure nuclear secrets.

Sources and Further Reading
Palaniappan, R. (2008). Two-stage biometric authentication method using EEG

Ashby et al. (2011). Low-cost EEG-based authentication

Chuang et al. (2013). User authentication based on ERP

Australian OAIC. Sensitive information guidelines
