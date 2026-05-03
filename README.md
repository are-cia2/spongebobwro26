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
For this competition, we have designed our own chassis and modelled it in Autodesk Fusion 360. We manufacture the vehicle and its components mainly using Fused Filament Fabrication (FFF) 3D printing with Polylactic Acid (PLA). We use off-the-shelf electrical components, such as motors from the LEGO MINDSTORMS EV3 kit, and sensors from various manufacturers, including ADD STUFF

### 3D printed body
We decided to design and 3D print the required parts in Autodesk Fusion360 instead of using parts like Lego parts, since this would give us much more freedom and flexibility in terms of designing the components we needed.

To design the 3D printed parts, we made use of Fusion 360, which combines Computer-Aided Design (CAD) and Computer-Aided Manufacturing (CAM) in one platform. This allows us to design and directly prepare our parts for printing. Fusion 360 is also known for its user-friendly interface, making it accessible for beginners while still offering advanced features for experienced users. The easy navigation of the software was useful to us as we could utilise it easily, and this also meant that our efforts could be replicated easily by interested beginners.

The use of FFF 3D printing is one of the most affordable 3D printing technologies. We can create multiple prototypes during our trial-and-error stage to build the most suitable and efficient robot. FFF 3D printing also has the versatility that allows the creation of complex geometries and customised designs that may otherwise not be easily achievable through traditional manufacturing methods. This is particularly useful for designing intricate custom parts of our robot. Additionally, the polylactic acid (PLA) filament used is strong and rigid, essential for structural components of our robot (e.g. the drive base structure, motor mounts).

Overall, Autodesk Fusion 350 and FFF 3D printing offer many benefits while building our robot for this competition.

ADD PICS + DESC

### Microcontroller
We decided to make use of the EVN Alpha to control the robot. Compared to the Lego Mindstorms EV3 and NXT controllers, the EVN Alpha has 64 holes on 5 sides of the controller, compared to 32 holes on 3 sides of the other controllers, making it much easier to mount other parts on the EVN Alpha. Furthermore, the EVN Alpha is also much more compact, allowing the robot’s movements to be much more precise and accurate. The EVN Alpha also features a USB-C Port that is used to charge the batteries and download code, which is much more convenient compared to the EV3 or other NXT Controllers. Lastly, this controller has a total of 16 I2C channels, which eliminates the 4-port constraint of other controllers like the EV3 brick.

### Movement

ADD STUFF

## Power & Sense Management
### Power Source
The robot is powered by 2 18650 cells. Each cell is rated for 4.2V when fully charged. They are connected in series to provide the EVN ALPHA with 8.4V. The motors run off unregulated battery power, consuming up to 780mA each at stall. There are 2 on-board regulators. The 3.3V regulator powers most of the system and can supply 3A. The current consumption of our 3.3V peripherals is under 1A. The 5V regulator only supplies the 5V rail and provides up to 3A. The only sensor we have attached to the 5V rail is the 2 Benewake sensors. While ranging, each sensor draws up to 600mA in a short pulse. This means that we are also well within the current capabilities of the regulator.

The cells we use are the Molicel P26A, featuring a maximum discharge current of 35A. At maximum load, our robot uses less than 2A, with the maximum contribution from the 2 medium motors. With a rated capacity of 2.6Ah, it can easily last more than an hour of continuous operation.

<img width="426" height="300" alt="v1_4_pt1" src="https://github.com/user-attachments/assets/c4eb360d-210f-4857-a4c8-51dcbb983d2b" />

### Sensors Used

## Obstacle Management

### Open Challenge

### Obstacle Challenge

### Functions Used

### Switchcase

### OpenMV Camera


