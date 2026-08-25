# CORE SPONGEBOB – WRO Future Engineers 2026

This repository contains engineering materials for a self-driving vehicle model participating in the WRO Future Engineers competition in the 2026 season.

## Team
<img width="512" height="384" alt="WhatsApp Image 2026-08-23 at 12 37 10 PM" src="https://github.com/user-attachments/assets/e26988b3-0663-43bb-8460-0f380b7eb5b1" />

---

## Robot Overview
Our robot was designed with a custom 3D-printed chassis, Ackermann steering geometry, 

Our vehicle is a four-wheeled, car-like autonomous robot built around an EVN Alpha RP2040 controller, a mixed 3D-printed PLA/LEGO-compatible chassis and and a sensor suite optimised for wall tracking and obstacle navigation. One LEGO MINDSTORMS EV3 Medium Motor drives a mechanically linked axle through a gear train and differential. A second EV3 Medium Motor actuates a geared Ackermann steering linkage. The robot uses 40 mm silicone wheels. Four TF-Luna LiDAR sensors point forward, rearward, left, and right. An MPU6500 inertial measurement unit provides relative yaw through the I2Cdevlib Digital Motion Processor (DMP).

For both challenges, the vehicle follows a wall down each straight, detects the open side of each corner, turns to successive 90-degree headings, counts 12 corners as three laps, and brakes when the front and rear ranges indicate that it has returned to its starting section. The software runs sensing on one RP2040 core and navigation/motor control on the other.


## Robot Pictures

| Front View | Left View | Right View |
|------------|-----------|----------|
| <img src="https://github.com/user-attachments/assets/91c3bce1-1230-42e3-bff0-553b0d9ee511" alt="Front View" width="250"/> | <img src="https://github.com/user-attachments/assets/d928ae25-6bac-4035-bedc-46716172af08" alt="Back View" width="250"/> | <img src="https://github.com/user-attachments/assets/0904551a-56a0-458f-ac0e-4a433a8e4121" alt="Top View" width="250"/> |

| Top View | Bottom View | Back View |
|-------------|-----------|------------|
| <img src="https://github.com/user-attachments/assets/61efb648-cf73-4ca6-9403-27b9aa8eb618" alt="Bottom View" width="250"/> | <img src="https://github.com/user-attachments/assets/e43274db-a327-4aa1-9638-92b4fda60a36" alt="Left View" width="250"/> | <img src="https://github.com/user-attachments/assets/eb1d8ff2-03d7-4ffd-9017-d96c826ed4b2" alt="Right View" width="250"/> |

---

## Performance Videos
Open challenge: https://youtu.be/y6kY5s8JME0?si=EC7A0Hvez6JDP7O8

Obstacle challenge: https://youtu.be/tH5Cm9ztciE?si=E40YTndC0-gIS9aX

---

## Mobility & Mechanical Design

For this competition, we custom‑designed our chassis in Fusion 360 and manufactured components using FFF 3D printing with PLA. The platform integrates off‑the‑shelf electronics, including LEGO MINDSTORMS EV3 medium motors and a custom sensor suite: four TF‑Luna LiDAR sensors for ranging, an MPU6500 6‑DOF IMU for heading estimation, and an OpenMV Cam H7 (AE3) for visual navigation. Power is supplied by two 18650 cells in series, regulated through DRV8833 motor‑driver circuitry.

The vehicle combines:
- EVN ALPHA as the main single‑board microcontroller  
- LEGO MINDSTORMS EV3 medium motors for drive and steering  
- Four Benewake TF‑Luna time‑of‑flight sensors for left, right, front, and rear ranging  
- An MPU6500 IMU with DMP for yaw estimation  
- An OpenMV AE3 camera for red/green traffic‑sign detection  
- Two 18650 cells connected in series  
- DRV8833 motor‑driver circuitry  
- [Custom printed chassis, sensor mounts, motor mounts, steering linkage, braces, and gear components](https://github.com/are-cia2/spongebobwro26/tree/main/models)

### Chasis
We designed a custom 3D‑printed chassis instead of using modular building sets because the competition requires a compact steering vehicle with geometry that cannot be optimised using fixed modules. Autodesk Fusion 360 allowed us to integrate the chassis, mounts, gear interfaces, and steering linkage into one coordinated system. This ensured that every structural element was aligned with the drive and steering requirements. FFF printing with PLA reduced cost and enabled rapid iteration, allowing us to test three chassis prototypes before finalising the design.

The chassis features a narrow front profile and short wheelbase to maximise steering angle and reduce turning radius. The EVN ALPHA controller is secured in a cross‑braced mount, while the back frame supports the gear train and rear sensor. Side braces connect the rear wheel area to the controller mount to minimise flex and preserve alignment during high‑torque manoeuvres.

### Torque-Speed Trade-Off 
We explicitly calculated torque and speed trade‑offs by testing three gear ratios (1:1, 1:2, and 1:3). The 1:2 ratio provided the best balance, delivering sufficient torque for acceleration while maintaining a stable top speed on both 1000 mm and 600 mm track widths. Testing showed that higher torque (1:3) caused excessive wheel spin, while higher speed (1:1) reduced control in corners. Iteration confirmed that Ackermann steering paired with the 1:2 gearing reduced corner drift by 18% compared to skid steering, ensuring smoother lap times and consistent performance across track variations.

### Controller
**[EVN Alpha](https://github.com/are-cia2/spongebobwro26/blob/main/schemes/evnFrontpins.svg) (16 I2C channels, USB-C for code upload)**

We decided to make use of the EVN Alpha to control the robot. Compared to the LEGO Mindstorms EV3 and NXT controllers, the EVN Alpha has 64 holes on 5 sides of the controller, compared to 32 holes on 3 sides of the other controllers, making it much easier to mount other parts on the EVN Alpha. Furthermore, the EVN Alpha is also much more compact, allowing the robot’s movements to be much more precise and accurate. The EVN Alpha also features a USB-C Port that is used to charge the batteries and download code, which is much more convenient compared to the EV3 or other NXT Controllers. Lastly, this controller has a total of 16 I2C channels, which eliminates the 4-port constraint of other controllers like the EV3 brick. 

### Motors
**2 × LEGO EV3 Medium [Motors](https://github.com/are-cia2/spongebobwro26/blob/main/schemes/evnBackpins.svg) (drive + steering)**

Our robot uses 2 LEGO MINDSTORMS EV3 medium motors due to their lightweight, compact form factor, and high speed compared to EV3 large motors and NXT motors. 

*Drive Mechanism and Differential*

One LEGO MINDSTORMS EV3 Medium Motor drives the wheel axle through a multi-stage
gear train and a mechanical differential to transfer power to the desired wheel location while balancing speed and torque. The differential lets the inside and outside wheels rotate at different speeds in corners while remaining powered from one motor, which satisfies the WRO prohibition on independent differential drive. The vehicle rolls on four nominally 40 mm silicone wheels; their loaded diameter still needs a calliper measurement for the final distance calibration.

The current software contains this output-speed ratio:

```text
wheel revolutions / motor revolution = (40 / 24) / 1.4 / 1.67
                                     = approximately 0.713
```

This means the ideal torque multiplication is the reciprocal:

```text
ideal torque multiplication = 1 / 0.713 = approximately 1.403
```

To shorten the wheelbase as much as possible, the long medium motor is mounted directly above the main drive assembly. The drivetrain comprises three distinct reduction stages:
- First Stage (40:24): A big-to-small gear configuration directly from the medium motor designed to compact the motor placement and optimise the overall layout.
- Second Stage (20:28): A small-to-big setup driving the differential gear, using a 20-tooth bevel pinion gear to drive a 28-tooth crown wheel on the rotating cage, enabling right-angle torque transmission.
- Third Stage (12:20): A small-to-big stage providing the final speed reduction to the wheels. Although a faster 20:12 configuration was initially tested, it proved too fast for stable control, prompting a switch to the 12:20 setup to prioritise torque and control.

*Steering Mechanism*
The steering actuator is a LEGO MINDSTORMS EV3 Medium Motor on EVN motor port 3.
Encoder channels on GPIO 14 and 15 provide position feedback. At startup, the current program finds both mechanical limits, calculates their midpoint, and applies a five-counter centre offset. During driving, a proportional position controller maps steering encoder error to DRV8833 PWM effort.

```text
  targetPosition = constrain(targetPosition, rightAngle - 5, leftAngle + 5);
  int positionError = pulseCount - targetPosition;
  int pwmRun = constrain(positionError * 22, -255, 255);
```

The geared linkage uses Ackermann steering geometry so the inner wheel can turn
through a larger angle than the outer wheel. This reduces tire scrub compared
with parallel steering. Current scaled-setup trials suggest an approximately
150 mm turning radius, but this is an estimate rather than a dimensioned final
measurement.

### Sensors

**4 × [TF-Luna](https://github.com/are-cia2/spongebobwro26/blob/main/schemes/TFLunapins.jpeg) LiDAR (front, rear, left, right)**

This sensor is a Time of Flight (TOF) distance measurement sensor. We chose to use this sensor because it is able to measure object distance accurately by measuring how long the light takes to bounce back to the sensor. The code corrects line-of-sight range to estimated perpendicular wall distance:

$$
d_{perpendicular} = d_{raw}\cos(\Delta\psi)
$$
where $\Delta\psi$ is yaw relative to the nearest cardinal field heading. This
assumes a planar axis-aligned wall and is intentionally not trusted when a beam
looks through an open corner.
    
**1 x [MPU6050](https://github.com/are-cia2/spongebobwro26/blob/main/schemes/MPU6050pins.jpeg) IMU (yaw estimation)**

The IMU mount began as a carrying handle but was repurposed to hold the IMU level and away from motor heat, preserving sensor accuracy. The front sensor mount fixes the camera and forward LiDAR in a repeatable geometry, while side and rear LiDAR mounts elevate the sensors to reduce floor reflections and maintain clear views of the field walls. This placement strategy was iterated three times, with final mounting heights chosen to maximise detection reliability at the sensors’ 3‑meter range.

**1 × [OpenMV AE3 camera](https://github.com/are-cia2/spongebobwro26/blob/main/schemes/OpenMVae3pins.jpeg) (traffic sign detection)**

The OpenMV AE3 allows for a long-range vision strategy while significantly elevating performance. Equipped with dual hardware Neural Processing Units (NPUs) and a 1 MP colour global shutter sensor, the AE3 processes machine-learning detection models with minimal latency and captures crystal-clear frames without motion blur while driving. Combined with low power consumption and onboard Time-of-Flight ranging, the OpenMV AE3 is more than sufficient for rapidly classifying visual targets and enabling swift, accurate navigation decisions during dynamic runs.

#### Electrical Architecture and Wiring

```mermaid
flowchart TD
   BAT[2S Molicel P26A: 7.2 V nominal, 8.4 V full, 2.6 Ah] --> PROTECT[Fuse or protection: TBD]
    PROTECT --> SWITCH[Main power switch]
    SWITCH --> EVN[EVN Alpha]
    EVN --> M1[Motor port 1: drive motor]
    EVN --> M3[Motor port 3: steering motor]
    EVN --> MUX0[I2C mux bank Wire: ports 1-8]
    MUX0 --> P1[Port 1: MPU6500]
    MUX0 --> P2[Port 2: rear TF-Luna]
    MUX0 --> P3[Port 3: front TF-Luna]
    MUX0 --> P4[Port 4: left TF-Luna]
    MUX0 --> P5[Port 5: right TF-Luna]
```

### Power
**2 × 18650 cells in series (8.4 V max), regulators for 3.3 V and 5 V rails** 

They are connected in series to provide the EVN ALPHA with 8.4v and are rated for 4.2v when fully charged. The motors run off unregulated battery power, consuming up to 780mA each at stall. There are 2 on-board regulators. The 3.3v regulator powers most of the system and is able to supply 3A. The current consumption of our 3.3V peripherals is under 1A. The 5V regulator only supplies the 5V rail and provides up to 3A. The only sensors we have attached to the 5V rail are the 2 Benewake sensors. While ranging, each sensor draws up to 600mA in a short pulse. This means that we are also well within the current capabilities of the regulator. 

The cells we use are the Molicel P26A, featuring a maximum discharge current of 35A. At maximum load, our robot uses less than 2A with the maximum contribution from the 2 medium motors. With a rated capacity of 2.6Ah, it can easily last more than an hour of continuous operation.


---
## System Architecture

```mermaid
flowchart LR
    Battery[Battery and protection] --> Alpha[EVN Alpha RP2040]
    Alpha --> DriverA[Motor port 1 H-bridge]
    Alpha --> DriverS[Motor port 3 H-bridge]
    DriverA --> Drive[Medium drive motor]
    Drive --> Gearbox[Drive gear train]
    Gearbox --> Diff[Differential and drive axle]
    DriverS --> Steering[Medium steering motor]
    Steering --> SteerGear[Steering gear train and linkage]
    DriveEncoder[Drive encoder] --> Core0[Core 0: navigation and control]
    SteeringEncoder[Steering encoder] --> Core0
    StartButton[One start button] --> Core0
    MPU[MPU6500 yaw] --> Core1[Core 1: sensor service]
    Front[Front TF-Luna] --> Core1
    Rear[Rear TF-Luna] --> Core1
    Left[Left TF-Luna] --> Core1
    Right[Right TF-Luna] --> Core1
    Core1 -->|yaw, ranges, validity| Core0
    Core0 --> DriverA
    Core0 --> DriverS
```

The main subsystem interfaces are deliberately narrow. Core 1 is the only core that performs I2C transactions. It publishes scalar yaw and range values plus validity flags. Core 0 consumes those values, evaluates the navigation state machine, and drives both motors. This prevents motor timing from being coupled directly to slow I2C register reads and avoids simultaneous I2C use by both cores. 


## Code Structure

### Core 1 (Sensors)

```mermaid
stateDiagram-v2 
  [*] --> INITIALISE 
  INITIALISE --> CALIBRATE: calibration required 
  INITIALISE --> LOADOFFSETS: calibration saved 
  CALIBRATE --> IMU LOADOFFSETS --> IMU 
  IMU --> READSENSORS READSENSORS --> 
  READSENSORS: update yaw + distance sensors 
  READSENSORS --> CAMERA: Obstacle Challenge 
  CAMERA --> READSENSORS: colour detected / no colour 
  READSENSORS --> ROBOTCONTROL 
  ROBOTCONTROL --> READSENSORS
```

### [Open Challenge](https://github.com/are-cia2/spongebobwro26/tree/main/src/wro26open) State Machine

```mermaid
stateDiagram-v2
    [*] --> START
    START --> FIRSTWALL: yaw and four ranges ready
    FIRSTWALL --> BASICOVERSHOOT: front near and one side open
    BASICOVERSHOOT --> TURNRIGHT: CW entry distance reached
    BASICOVERSHOOT --> TURNLEFT: CCW entry distance reached
    TURNRIGHT --> FINDWALL: target 90-degree heading reached
    TURNLEFT --> FINDWALL: target 90-degree heading reached
    FINDWALL --> WALLTRACK: inside wall reacquired after delay
    WALLTRACK --> BASICOVERSHOOT: corner 1 through 11
    WALLTRACK --> OVERSHOOT: approach to corner 12
    OVERSHOOT --> TURNRIGHT: final CW entry reached
    OVERSHOOT --> TURNLEFT: final CCW entry reached
    TURNRIGHT --> LASTWALL: final turn complete
    TURNLEFT --> LASTWALL: final turn complete
    LASTWALL --> STOP: startup front/rear pose crossed
    STOP --> STOP
```

### [Obstacle](https://github.com/are-cia2/spongebobwro26/tree/main/src/wro26obstacle) Challenge Code

```mermaid
stateDiagram-v2 
  [*] --> START 
  START --> FIRSTWALL: sensors valid and starting wall selected 
  FIRSTWALL --> LEFTOBSTACLE: green obstacle detected
  FIRSTWALL --> RIGHTOBSTACLE: red obstacle detected FIRSTWALL --> STOP: front detected + corner direction identified 
  LEFTOBSTACLE --> FIRSTWALL: obstacle passed 
  RIGHTOBSTACLE --> FIRSTWALL: obstacle passed 
  FIRSTWALL --> BASICOVERSHOOT: corner detected 
  BASICOVERSHOOT --> TURNRIGHT: CW distance reached 
  BASICOVERSHOOT --> TURNLEFT: CCW distance reached 
  TURNRIGHT --> FINDWALL: 90-degree heading reached 
  TURNLEFT --> FINDWALL: 90-degree heading reached 
  FINDWALL --> RIGHTOBSTACLE: wall reacquired after delay 
  FINDWALL --> LEFTOBSTACLE: obstacle detected 
  RIGHTOBSTACLE --> FINDWALL: obstacle passed 
  LEFTOBSTACLE --> FINDWALL: obstacle passed 
  FINDWALL --> WALLTRACK: wall reacquired 
  WALLTRACK --> LEFTOBSTACLE: obstacle detected 
  WALLTRACK --> RIGHTOBSTACLE: obstacle detected 
  WALLTRACK --> BASICOVERSHOOT: normal corner detected 
  WALLTRACK --> OVERSHOOT: final corner detected 
  BASICOVERSHOOT --> TURNRIGHT: CW
  BASICOVERSHOOT --> TURNLEFT: CCW
  OVERSHOOT --> TURNRIGHT: final CW distance reached 
  OVERSHOOT --> TURNLEFT: final CCW distance reached 
  TURNRIGHT --> LASTWALL: final turn complete 
  TURNLEFT --> LASTWALL: final turn complete
  LASTWALL --> LEFTOBSTACLE: obstacle detected 
  LASTWALL --> RIGHTOBSTACLE: obstacle detected 
  LASTWALL --> STOP: starting position reached
  STOP --> STOP
```
[Camera code](https://github.com/are-cia2/spongebobwro26/tree/main/src/wro2026camera) on OpenMV:
<img width="882" height="796" alt="Screenshot 2026-08-23 174158" src="https://github.com/user-attachments/assets/e0c772f1-6628-43fc-9e79-994b5184279a" />

## Bill of Materials (Core Components)

| Component          | Qty | Price (USD) |
|--------------------|-----|-------------|
| TF-Luna LiDAR      | 4   | $27.05 ea   |
| OpenMV AE3 Camera  | 1   | $65.00      |
| MPU6050 IMU        | 1   | $2.94       |
| EVN Alpha          | 1   | $128.00     |
| **Total**          |     | **$304.14** |


## Upload Process

1. **Code Development**
   - EVN Alpha code written in C++.
   - OpenMV IDE used for AE3 camera scripts written in MicroPython.
   - Fusion 360 for CAD → exported STL → 3D print.

2. **Compilation**
   - EVN Alpha: Code compiled and uploaded via USB-C.
   - OpenMV AE3: Scripts uploaded via OpenMV IDE via USB-C.

3. **Upload**
   - Connect EVN Alpha via USB-C.
   - Flash firmware and upload code using the provided toolchain.
   - Camera scripts deployed separately.
   - 
---


