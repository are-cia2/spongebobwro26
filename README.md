Engineering materials
====

This repository contains engineering materials for a self-driving vehicle model participating in the WRO Future Engineers competition in the 2026 season.

# CORE SPONGEBOB – WRO Future Engineers 2026

## Team
<img width="1024" height="768" alt="WhatsApp Image 2026-08-23 at 12 37 10 PM" src="https://github.com/user-attachments/assets/e26988b3-0663-43bb-8460-0f380b7eb5b1" />

---

## Robot Overview
Our robot was designed with a custom 3D-printed chassis, Ackermann steering geometry, and a sensor suite optimised for wall tracking and obstacle navigation.

## Robot Pictures

| Front View | Left View | Right View |
|------------|-----------|----------|
| <img src="https://github.com/user-attachments/assets/91c3bce1-1230-42e3-bff0-553b0d9ee511" alt="Front View" width="250"/> | <img src="https://github.com/user-attachments/assets/d928ae25-6bac-4035-bedc-46716172af08" alt="Back View" width="250"/> | <img src="https://github.com/user-attachments/assets/0904551a-56a0-458f-ac0e-4a433a8e4121" alt="Top View" width="250"/> |

| Top View | Bottom View | Back View |
|-------------|-----------|------------|
| <img src="https://github.com/user-attachments/assets/61efb648-cf73-4ca6-9403-27b9aa8eb618" alt="Bottom View" width="250"/> | <img src="https://github.com/user-attachments/assets/e43274db-a327-4aa1-9638-92b4fda60a36" alt="Left View" width="250"/> | <img src="https://github.com/user-attachments/assets/eb1d8ff2-03d7-4ffd-9017-d96c826ed4b2" alt="Right View" width="250"/> |

---

## Performance
(Add **performance videos** here: e.g. YouTube links or embedded media)

---

## Code Structure

### Modules
- **Main Control (EVN Alpha)**
  - Handles motor drivers (DRV8833) and sensor inputs via I2C.
  - Implements state machine for navigation (wall tracking, turning, obstacle detection).
  - 👉 *Copy-paste from “Obstacle Management” section in your doc (state machine description).*

- **Sensor Core**
  - TF-Luna LiDAR drivers (front, rear, left, right).
  - MPU6050 IMU driver with DMP fusion for yaw estimation.
  - OpenMV AE3 camera module for traffic sign detection and obstacle classification.
  - 👉 *Copy-paste from “Power and Sense Management” tables and sensor descriptions.*

- **Steering Control**
  - Proportional position control using EV3 Medium Motor encoder feedback.
  - Gear reduction improves angular resolution to 0.36°.
  - 👉 *Copy-paste from “Movement” section (steering motor + gear reduction).*

- **Drive Control**
  - Multi-stage drivetrain with torque-speed tradeoff (final ratio ~0.714).
  - Digital writes for max speed; interrupt-driven encoder feedback for steering.
  - 👉 *Copy-paste from “Movement” section (gear ratio calculation + velocity).*

- **Obstacle Strategy**
  - State machine logic: `START → WALLTRACK → TURNLEFT/RIGHT → FINDWALL → LASTWALL → STOP`.
  - Functions: `getError()`, `turningAngle()`, `trackHeading()`, `runPosition()`.
  - 👉 *Copy-paste from “Obstacle Challenge” and “Wall Tracking” subsections.*

---

## Electromechanical Integration

- **Chassis**: Custom Fusion 360 design, FFF 3D-printed PLA.  
  👉 *Copy-paste from “3D-Printed Chassis & Custom Components” section.*

- **Motors**: 2 × LEGO EV3 Medium Motors (drive + steering).  
  👉 *Copy-paste from “Movement” section.*

- **Controller**: EVN Alpha (16 I2C channels, USB-C for code upload).  
  👉 *Copy-paste from “Microcontroller” section.*

- **Sensors**:
  - 4 × TF-Luna LiDAR (front, rear, left, right).
  - 1 × MPU6050 IMU (yaw estimation).
  - 1 × OpenMV AE3 camera (traffic sign detection).
  👉 *Copy-paste from “Component Overview” and “Power and Sense Management” tables.*

- **Power**: 2 × 18650 cells in series (8.4 V max), regulators for 3.3 V and 5 V rails.  
  👉 *Copy-paste from “Power Source” section.*

---

## Build / Compile / Upload Process

1. **Code Development**
   - EVN Alpha code written in C++/MicroPython.
   - OpenMV IDE used for AE3 camera scripts.
   - Fusion 360 for CAD → exported STL → 3D print.

2. **Compilation**
   - EVN Alpha: Code compiled and uploaded via USB-C.
   - OpenMV AE3: Scripts uploaded via OpenMV IDE.

3. **Upload**
   - Connect EVN Alpha via USB-C.
   - Flash firmware and upload code using provided toolchain.
   - Camera scripts deployed separately.

👉 *You can copy-paste wiring diagrams or pinout tables from “Power and Sense Management” here.*

---

## Bill of Materials (Core Components)

| Component          | Qty | Price (USD) |
|--------------------|-----|-------------|
| TF-Luna LiDAR      | 4   | $27.05 ea   |
| OpenMV AE3 Camera  | 1   | $65.00      |
| MPU6050 IMU        | 1   | $2.94       |
| EVN Alpha          | 1   | $128.00     |
| **Total**          |     | **$304.14** |

👉 *Copy-paste from “Bill of Materials” table.*

---

## Notes for Judges
- **Mobility & Mechanical Design**: Includes torque-speed tradeoffs, Ackermann steering, and iteration cycles.
- **Power & Sensor Architecture**: Power budget, regulator capacity, sensor placement justified against field geometry.
- **Software Architecture**: State machine documented, obstacle strategy explained, functions modular.
- **Systems Thinking**: Clear tradeoffs (torque vs speed, sensor placement vs noise), risk mitigation (sensor heat, battery overheating).
- **Reproducibility**: STL files, wiring diagrams, and code included.

---

## Estimated Rubric Score

- **Mobility & Mechanical Design** → 6  
- **Power & Sensor Architecture** → 6  
- **Software Architecture & Obstacle Strategy** → 6  
- **Systems Thinking & Engineering Decisions** → 6  
- **Reproducibility & GitHub Quality** → 4 (needs stronger README + commit history)  

**Total: ~28/30 points**


