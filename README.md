# Didge - A Digital Gearbox
## What is a digital gearbox?
A *digital gearbox* is an electronic device which emulates a gearbox but with
dynamically adjustable gear ratios. For example, if it the ratio is set to 1/3,
the output shaft should rotate 1/3 rotations for every rotation of the input shaft.

Basically it aims to replace some type of this:

<img width=300 src=https://www.practicalmachinist.com/vb/attachments/f38/55156d1342154633-ot-repairing-lathe-gearbox-teeth-img_3978.jpg />

with something like this (input shaft, sensor and digital gearbox not shown):

<img width=300 src=https://www.photomacrography.net/forum/userpix/679_CU_of_Stepper_timing_belts_and_pulleys_1.jpg />

## Why
Unlike a physical gearbox with actual gears, a digital gearbox has no practical 
limit on how many gear ratios it can be configured to. No gears to wear down or break. 

**Didge** digital gearbox is implemented on a low cost 32 bit microcontroller
board. Thanks to the clever use of an algorithm combined with utilization of the
hardware resource in the microcontroller, it can emulates a real gearbox with very low
latency (<80 nanoseconds) and is able to generate a smooth motion output regardless of 
the selected gear ratio. No floating point representation or scaling errors. 
You can read about [how it works here](https://github.com/prototypicall/Didge/blob/master/doc/How.md)

It is implemented with the intent to replace the physical gearbox in a metal cutting [lathe](https://en.wikipedia.org/wiki/Lathe) but can be adapted for other uses where a dynamically configurable gear reduction is needed, e.g. for synchronized 
motion applications.

You can see the source code in the [firmware](https://github.com/prototypicall/Didge/tree/master/firmware) folder.

## Status
The main gearbox emulation, including smooth step generation and support for
arbitrary gear ratios, is working.

Test setup is composed of a small NEMA 17 size stepper motor driven by a
[StepStick](https://reprap.org/wiki/StepStick) stepper driver board (a similar
combination is used in many hobby grade 3D printers to control the print head
and the platform) and a 2400 pulse/rev rotary encoder.

Below is a video with brief description of the test setup and some footage of 
operation, driving the stepper motor close to its limits at 1200+ RPM.

[![Video](https://img.youtube.com/vi/SvH-SeT9NUI/0.jpg)](https://www.youtube.com/watch?v=SvH-SeT9NUI)

Main screen:

![image](https://user-images.githubusercontent.com/61201064/81103576-167a3d00-8ec6-11ea-9a63-47692b28831e.png)

Thread selection screen:

![image](https://user-images.githubusercontent.com/61201064/81103740-550ff780-8ec6-11ea-89ad-c3a49410a3ab.png)

Settings screen:

![image](https://user-images.githubusercontent.com/61201064/81104130-e8e1c380-8ec6-11ea-884f-5c273bb3ed99.png)


## Planned Features

- *Graphical user interface combined with physical buttons:* See all the relevant
    info on the screen and operate with the touch screen or with buttons.
- *Reconfigurable:* All the settings needed to use on a specific machine and motor
    configuration are adjustable during operation without needing source code changes.
    Some of these settings are:
    * Encoder pulse count (per revolution) and gearing (to the spindle)
    * Encoder signal filtering
    * Lead screw pitch (metric or imperial)
    * Steps per revolution, micro steps and gearing to the lead screw.
    * Step pulse length and direction hold time (maybe different for each 
        stepper/servo driver)
    * Acceleration, deceleration and maximum speed for the stepper
    * Polarities for step and direction signals
- *User defined threads*
- *Travel control:* Starting and stopping the lead screw to control thread length
    or turn down to a shoulder, in synchronization with the input (i.e the spindle) 
    or stalling the stepper/servo motor.
- *Jogging:* Move the carriage for precise positioning.
- *Safety:* 
    * Keep the motor in valid speed range, disengage if exceeded. 
    * E-STOP. Input and output to integrate with the rest of the system
    * Motor position tracking.
- *Dual motor drive:* Ability use a second motor, attached to a pinion rack mechanism
    for example, to be used for feed mode.
- *Multi-start thread support*
