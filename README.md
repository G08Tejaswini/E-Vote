# 🗳️ Fingerprint Based Electronic Voting Machine

A biometric electronic voting machine built using Arduino Uno and an R307 fingerprint sensor to provide secure voter authentication and prevent duplicate voting.

---

## 📌 Overview

This project was developed as part of an embedded systems and electronics group project focused on improving voting security through biometric authentication.

The system verifies voters using fingerprint recognition before allowing them to cast a vote. Once authenticated, the voter can select a candidate using push buttons, and the vote is securely stored while preventing duplicate voting attempts.

The project combines embedded systems, biometric verification, hardware interfacing, and user feedback mechanisms into a low-cost prototype suitable for educational and small-scale election environments.

---

## ✨ Features

- Fingerprint-based voter authentication
- Prevents duplicate voting
- LCD-guided voting interface
- Candidate selection using push buttons
- Vote tally display
- Audio feedback using buzzer
- Offline embedded implementation
- EEPROM-based vote storage

---

## 🛠️ Hardware Components

- Arduino Uno
- R307 Fingerprint Sensor
- 16x2 LCD Display
- Push Buttons
- LEDs
- Buzzer
- EEPROM Memory
- Resistors & Wiring Components

---

## 🧠 How It Works

1. System initializes all connected hardware components
2. Voter fingerprints are enrolled into the database
3. User scans fingerprint for authentication
4. If verified, voting is enabled
5. Vote is recorded securely
6. Duplicate voting attempts are blocked
7. Results can be displayed on the LCD

---

## 🔌 Circuit Connections

### Fingerprint Sensor → Arduino UNO
- VCC → 5V
- GND → GND
- TX → Pin 2
- RX → Pin 3

### LCD 16x2 → Arduino UNO
- RS → 13
- E → 12
- D4 → 11
- D5 → 10
- D6 → 9
- D7 → 8

### Voting Buttons
- Candidate 1 → Pin 4
- Candidate 2 → Pin 5
- Candidate 3 → Pin 6

---

## 📂 Project Structure

```txt
e-voting-machine/
│
├── voting_machine.ino
├── report.pdf
├── presentation.pptx
├── images/
├── LICENSE 
└── README.md
```

---

## 🚧 Challenges Faced

- LCD contrast instability
- Fingerprint sensor communication issues
- EEPROM memory management
- Fingerprint enrollment consistency
- Logic-level mismatch between Arduino and sensor

These issues were resolved through hardware debugging, voltage divider implementation, and improved memory organization.

---

## 🚀 Future Scope

- Support for more candidates
- Wireless result transmission
- RFID + fingerprint dual authentication
- Touchscreen interface
- Cloud/database integration
- Encrypted fingerprint storage
- Solar-powered deployment
- Real-time clock scheduling

---

## 📊 Performance Highlights

- Accurate biometric authentication
- Duplicate voting prevention
- Fast response time
- Low-cost implementation
- User-friendly LCD interface
- Secure vote storage

---

## 🎯 Project Objectives

- Eliminate impersonation and duplicate voting
- Build a secure biometric EVM prototype
- Create an accessible voting interface
- Implement one-person-one-vote logic
- Develop a scalable embedded voting system

---

## 👥 Contributors

- **Ganti Tejaswini** — Electronics and Communication Engineering student
- **BHIMAVARAPU ADITI** —  Electronics and Communication Engineering student
- **NIYATI JAIN** — Aerospace Engineering student
- **GORTHI SAI SATWIK** — Electronics and Communication Engineering student
- **CHILAKAMARTHI VENKATA SAI SANJANA** — Aerospace Engineering student

---

## 📄 License

MIT License
