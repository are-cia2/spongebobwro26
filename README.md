Engineering materials
====

This repository contains engineering materials of a self-driven vehicle's model participating in the WRO Future Engineers competition in the season 2026.

## Content DELETE THIS EVENTUALLY

* `t-photos` contains 2 photos of the team (an official one and a funny photo with all team members)
* `v-photos` contains 6 photos of the vehicle (from every side, from top and bottom)
* `video` contains the video.md file with the link to a video where a driving demonstration exists
* `schemes` contains one or several schematic diagrams in the form of JPEG, PNG or PDF of the electromechanical components illustrating all the elements (electronic components and motors) used in the vehicle and how they connect.
* `src` contains the code of control software for all components which were programmed to participate in the competition
* `models` is for the files for models used by 3D printers, laser cutting machines and CNC machines to produce the vehicle elements. If there is nothing to add to this location, the directory can be removed.
* `other` is for other files, which can be used to understand how to prepare the vehicle for the competition. It may include documentation how to connect to an SBC/SBM and upload files there, datasets, hardware specifications, communication protocols descriptions, etc. If there is nothing to add to this location, the directory can be removed.

## Introduction

_This part must be filled by participants with the technical clarifications about the code: which modules the code consists of, how they are related to the electromechanical components of the vehicle, and what the process is to build/compile/upload the code to the vehicle’s controllers._

## The Team

## The Robot


### Pictures

## Performance Videos

### Challenge 1

### Challenge 2

## Mobility Management
### Vehicle Design & Hardware Overview
For this competition, we custom-designed our chassis in Autodesk Fusion 360 and manufactured the components primarily using Fused Filament Fabrication (FFF) 3D printing with Polylactic Acid (PLA). The platform integrates off-the-shelf electronics, including LEGO MINDSTORMS EV3 medium motors and a custom sensor suite: four TF-Luna LiDAR sensors for ranging, an MPU6500 6-DOF IMU for heading estimation, and an OpenMV Cam H7 (AE3) for visual navigation.

### 3D-Printed Chassis & Custom Components
Rather than relying on modular building sets like LEGO, we opted to design and 3D print our custom components. This approach gave us complete freedom to tailor every part to our specific geometric and functional requirements.

We chose Autodesk Fusion 360 because it unifies Computer-Aided Design (CAD) and Computer-Aided Manufacturing (CAM) within a single workflow. Its intuitive interface allowed us to rapidly move from initial concepts to print-ready models. Beyond streamlining our process, using accessible software ensures that our design methodology can easily be replicated or modified by other students entering the field.

FFF 3D printing with PLA offered several key advantages for our development cycle, such as:
Cost-effective iteration
Low material costs enabled us to rapidly prototype, test, and refine multiple iterations during our trial-and-error phase.
Complex geometries
FFF allowed us to produce intricate, custom geometries—such as specialised motor mounts and sensor housings—that would be difficult or costly to produce using traditional manufacturing techniques.
Structural rigidity
PLA provided the high tensile strength and stiffness required for high-stress structural elements like our drive base.

Ultimately, combining Fusion 360 with FFF printing gave us an agile, cost-effective, and highly customised foundation for our competition robot.

### Microcontroller
We decided to make use of the EVN Alpha to control the robot. Compared to the Lego Mindstorms EV3 and NXT controllers, the EVN Alpha has 64 holes on 5 sides of the controller, compared to 32 holes on 3 sides of the other controllers, making it much easier to mount other parts on the EVN Alpha. Furthermore, the EVN Alpha is also much more compact, allowing the robot’s movements to be much more precise and accurate. The EVN Alpha also features a USB-C Port that is used to charge the batteries and download code, which is much more convenient compared to the EV3 or other NXT Controllers. Lastly, this controller has a total of 16 I2C channels, which eliminates the 4-port constraint of other controllers like the EV3 brick. 
### Movement

ADD STUFF

## Power & Sense Management
### Power Source
The robot is powered by 2 18650 cells. Each cell is rated for 4.2v when fully charged. They are connected in series to provide the EVN ALPHA with 8.4v. The motors run off unregulated battery power, consuming up to 780mA each at stall. There are 2 on-board regulators. The 3.3v regulator powers most of the system, and is able to supply 3A. The current consumption of our 3.3V peripherals is under 1A. The 5V regulator only supplies the 5V rail and provides up to 3A. The only sensor we have attached to the 5V rail are the 2 Benewake sensors. While ranging, each sensor draws up to 600mA in a short pulse. This means that we are also well within the current capabilities of the regulator. 

The cells we use are the Molicel P26A featuring a maximum discharge current of 35A. At maximum load, our robot uses less than 2A with the maximum contribution from the 2 medium motors. With a rated capacity of 2.6Ah, it can easily last more than an hour of continuous operation.

<img width="426" height="300" alt="v1_4_pt1" src="https://github.com/user-attachments/assets/c4eb360d-210f-4857-a4c8-51dcbb983d2b" />

### Sensors Used
### TF-Luna

We used four sensors in total, in the four cardinal directions.

This sensor is a Time of Flight (TOF) distance measurement sensor. We chose to use this sensor because it is able to measure object distance accurately by measuring how long the light takes to bounce back to the sensor. It is able to measure a target object that is up to 8m away with 1cm resolution, which is sufficient for wall tracking the inner wall of the field. This sensor is also very compact, which would make it very easy to incorporate into our robot design. It offers quick distance measurements, which is beneficial in terms of making turns at corners quickly.

#### Power Consumption 
Per Sensor: Average Current Draw = 70mA, Peak Current Draw = 150mA 

Total: Average Current Draw = 280mA, Peak Current Draw = 600mA

### OpenMV AE3 Camera

The camera is placed at the front of the robot. We previously used the front-mounted OpenMV H7 camera to detect traffic lights and parking spaces from a distance, giving our robot the lead time needed to make proactive steering decisions rather than relying on short-range colour sensors. Upgrading to the OpenMV AE3 preserves this long-range vision strategy while significantly elevating performance. Equipped with dual hardware Neural Processing Units (NPUs) and a 1 MP colour global shutter sensor, the AE3 processes machine-learning detection models with minimal latency and captures crystal-clear frames without motion blur while driving. Combined with low power consumption and onboard Time-of-Flight ranging, the OpenMV AE3 is more than sufficient for rapidly classifying visual targets and enabling swift, accurate navigation decisions during dynamic runs.

#### Power Consumption
While Idle: 24mA, Active Processing: 50mA to 60mA

### MPU6050

This sensor is an inertial measurement unit (IMU) that combines a 3‑axis gyroscope and a 3‑axis accelerometer. We chose to use this sensor because it is able to measure the robot’s orientation (angles and heading) accurately by tracking angular speed and displacement. Unlike a magnetic compass, which is affected by magnetic interference from motors, wiring, and the competition field, the MPU6050 provides stable readings that are not influenced by external magnetic fields.

The gyroscope measures angular speed, and orientation can be calculated as angular displacement = angular speed × time. However, since speed is not constant, small errors can accumulate over time. Normally, this makes gyroscopes less reliable if used alone. The MPU6050 solves this problem by combining gyro and accelerometer data inside its Digital Motion Processor (DMP). With library support, the chip itself calculates the heading, reducing drift and offloading computation from the robot’s main controller. This makes the angle calculation more precise and efficient.

#### Power Consumption
Normal Operating Current: 3.8 mA, Gyroscope + Accelerometer, With DMP: 3.9 mA, Gyroscope Only: 3.6 mA, Accelerometer Only: 500 µA , Full-Chip Idle Mode: 5 µA

Low-Power Accelerometer: 1.25 Hz = 10 µA, 5 Hz = 20 µA, 20 Hz = 70 µA, 40 Hz = 140 µA

### Bill of Materials (USD)
TF-Luna: $27.05 x4

OpenMV AE3 Camera: $65.00 x1

MPU6050: $2.94 x1

EVN ALPHA: $128.00 x1

Total: $304.14




## Obstacle Management

### Open Challenge

### Obstacle Challenge

### Functions Used

### Switchcase

### OpenMV Camera


