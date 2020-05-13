# Didge - Digital Gearbox

A *digital gearbox* is an electronic device which emulates a gearbox but with
dynamically adjustable gear ratio. It is a setup in which an sensor is coupled
to an input shaft and a motor is coupled to an output shaft. The sensor and
motor driver are connected to the *device* to track the input shaft and drive the
motor such that the gear ratio is held. For example, if it the ratio is set to 1/3,
the output shaft should rotate 1/3 rotations for every rotation of the input shaft.

Basically it aims to replace some type of this:

<img width=300 src=https://www.practicalmachinist.com/vb/attachments/f38/55156d1342154633-ot-repairing-lathe-gearbox-teeth-img_3978.jpg />

with something like this (input shaft, sensor and digital gearbox not shown):

<img width=300 src=https://www.photomacrography.net/forum/userpix/679_CU_of_Stepper_timing_belts_and_pulleys_1.jpg />

*Unlike a physical gearbox with actual gears, a digital gearbox has no practical 
limit on how many gear ratios it can be configured to. No gears to wear down. But
it also breaks the link between the shafts. So, it does not allow power transfer
from the input shaft to the output shaft. This rules out some applications.*

**Didge** is a digital gearbox, implemented on a low cost 32 bit microcontroller
board (called the STM32 "Blue pill"). Generates low latency (<80 nanosecond)
smooth motion regardless of gear ratio. No floating point operations and scaling error. 
You can read about [how it works here](https://github.com/prototypicall/Didge/blob/master/doc/How.md)

It is implemented with the intent to replace the physical gearbox in a lathe that 
drives the lead screw but can be adapted for other uses where a dynamically 
configurable gear reduction is needed, e.g. for synchronized motion applications.
A [MPG](https://en.wikipedia.org/wiki/Manual_pulse_generator) device is a good example.

## Features
Below are some of the planned features:

* *Graphical user interface combined with physical buttons:* See all the relevant
    info on the screen and operate with the touch screen or with buttons.
* *Reconfigurable:* All the settings needed to use on a specific machine and motor
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

You can see the source code in the firmware folder.

## Status
The main gearbox emulation, including smooth step generation and support for
arbitrary gear ratios, is working.

Test setup is composed of a small NEMA 17 size stepper motor driven by a
[StepStick](https://reprap.org/wiki/StepStick) stepper driver board (a similar
combination is used in many hobby grade 3D printers to control the print head
and the platform) and a 2400 pulse/rev rotary encoder.

Below is a video with brief description and footage of the setup driving the 
stepper, close to its limits, at 1200+ RPM with a 1 to 1 rotational ratio.

[![Video](https://img.youtube.com/vi/SvH-SeT9NUI/0.jpg)](https://www.youtube.com/watch?v=SvH-SeT9NUI)

Basic user interface is there. Graphic display shows the RPM, selected pitch and 
allows selecting among a sample set of pitches (metric and imperial) on the fly.
Start/stop, direction change and feed mode are to be implemented next.

Main screen:

![image](https://user-images.githubusercontent.com/61201064/81103576-167a3d00-8ec6-11ea-9a63-47692b28831e.png)

Thread selection (list is incomplete):

![image](https://user-images.githubusercontent.com/61201064/81103740-550ff780-8ec6-11ea-89ad-c3a49410a3ab.png)

Settings (firmware part to be implemented)

![image](https://user-images.githubusercontent.com/61201064/81104130-e8e1c380-8ec6-11ea-884f-5c273bb3ed99.png)


## TODO
Add feed mode which basically provides very high reductions and an increment, 
decrement interface to control.

Add controlled start, stop, pause, resume and direction changes (with acceleration)

Settings/configuration interface. Non-volatile storage (using internal 
flash memory or an external EEPROM or SD card).

Implement travel control.

Graphical user interface and button/keypad final design. Enclosure selection.

Multi start thread support.

Remaining features.
