Engineering materials
====

This repository contains engineering materials of a self-driven vehicle's model participating in the WRO Future Engineers competition in the season 2026.

## Content DELETE THIS EVENTUALLY

* `t-photos` contains 2 photos of the team (an official one and one funny photo with all team members)
* `v-photos` contains 6 photos of the vehicle (from every side, from top and bottom)
* `video` contains the video.md file with the link to a video where driving demonstration exists
* `schemes` contains one or several schematic diagrams in form of JPEG, PNG or PDF of the electromechanical components illustrating all the elements (electronic components and motors) used in the vehicle and how they connect to each other.
* `src` contains code of control software for all components which were programmed to participate in the competition
* `models` is for the files for models used by 3D printers, laser cutting machines and CNC machines to produce the vehicle elements. If there is nothing to add to this location, the directory can be removed.
* `other` is for other files which can be used to understand how to prepare the vehicle for the competition. It may include documentation how to connect to a SBC/SBM and upload files there, datasets, hardware specifications, communication protocols descriptions etc. If there is nothing to add to this location, the directory can be removed.

## Introduction

_This part must be filled by participants with the technical clarifications about the code: which modules the code consists of, how they are related to the electromechanical components of the vehicle, and what is the process to build/compile/upload the code to the vehicle’s controllers._

## The Team

## The Robot

### Pictures

## Performance Videos

### Challenge 1

### Challenge 2

## Mobility Management

## Power & Sense Management
### Power Source
The robot is powered by 2 18650 cells. Each cell is rated for 4.2v when fully charged. They are
connected in series to provide the EVN ALPHA with 8.4v. The motors run off unregulated battery
power, consuming up to 780mA each at stall. There are 2 on-board regulators. The 3.3v regulator
powers most of the system, and is able to supply 3A. The current consumption of our 3.3V
peripherals is under 1A. The 5V regulator only supplies the 5V rail and provides up to 3A. The
only sensor we have attached to the 5V rail are the 2 Benewake sensors. While ranging, each
sensor draws up to 600mA in a short pulse. This means that we are also well within the current
capabilities of the regulator.
The cells we use are the Molicel P26A featuring a maximum discharge current of 35A. At
maximum load, our robot uses less than 2A with the maximum contribution from the 2 medium
motors. With a rated capacity of 2.6Ah, it can easily last more than an hour of continuous
operation.
