# The Microcontoller
The firmware was developed for a STM32F103 microcontroller. It is has an ARM Cortex
M3 CPU clocked at 72 MHz. 64 KB flash (ROM) and 20 KB of RAM. It is readily available as 
"[Blue pill](https://stm32-base.org/boards/STM32F103C8T6-Blue-Pill.html)" boards, 
very low cost (less than $5). It also has many 5V tolerant pins which makes it easier to
connect to various devices.

<img width=600 src=https://solovjov.net/reblag.dk/The-Generic-STM32F103-Pinout-Diagram.jpg />

**TODO** Add pinout/connection table.
[Reference manual](https://www.st.com/resource/en/reference_manual/cd00171190.pdf) for 
figuring out how all the peripherals are to be configured.

# Rotary encoder
Any incremental rotary encoder that can operate at 3.3V to 5V should work but there
are different types of them with varying characteristics. In the test setup I am using 
a 600 CPR (counts per revolution) incremental optical rotary encoder which generates 
2400 PPR (pulses per revolution). It's has model number LPD 3806. There are higher
resolution models with similar cost.

All encoders have some limit on how fast they can correctly track the rotation, limiting 
the maximum rotational speed (i.e RPM). So keep these in mind while choosing.

Since the internal pull-up resistors of the microcontroller are quite large and
the encoder I am using has [open collector outputs](https://en.wikipedia.org/wiki/Open_collector), 
I am using external pull-up resistors to increase noise immunity. 5k - 10k should 
be fine. There is built-in filtering available in the microcontroller to deal with
noisy encoders if needed. This will is one of the configuration options.

# Drive Motor
Any stepper motor or servo motor controller that takes Step and Direction signals
should work. Parameters like step pulse duration and direction signal hold time
are configurable (will be configurable through the user interface as well). There
are hybrid stepper motor systems with closed loop control as well.

# Graphic LCD
I am currently using 2.4" [Nextion HMI display](https://nextion.tech/). It is a 
programmable graphic display module and reduces the job of the microcontroller 
interfacing to it to just sending and receiving UART messages. There are multiple
sizes, 2.4" being smallest and series with different capabilities. 2.4" feels like
it could be a bit larger for better usability.
 
It provides an editor to relatively easily bring up a graphical user interface. 
However the interface elements are quite limited and the protocol for interfacing
is not well designed or documented. So it takes significant effort to make it 
behave like proper graphical user interface in terms of interactivity. For example,
there is no combo box or list box element, nor radio buttons behave as you'd expect, etc.
