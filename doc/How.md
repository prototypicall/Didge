# The setup
The input shaft connected to a rotary encoder that measures its rotational movement.
Output shaft is connected to a motor (a stepper motor or servo motor controller 
taking in Step and Direction signals in our case) so that we can control the output 
shaft. Our device is connected to the encoder and the motor driver.

* Rotary encoder generates signals that correspond to the amount movement of the 
input axis. In our case it is of the quadrature encoder variety which provides
directional position information. Depending on the model selected, it has a 
fixed resolution **E**, which defines how many pulses it will generate for a full
revolution. So, each pulse corresponds to (1/E) of a full revolution.
* Stepper motor (or a servo motor) also has a resolution, that is the number of
steps per revolution. Most drivers for these motors have configurable number of 
micro steps to increases that resolution ([with trade-offs](https://hackaday.com/2016/08/29/how-accurate-is-microstepping-really/) )
Let's define **S** as the number of steps pulses needed to take for making a full 
revolution.

# Mode of Operation
The device will monitor the pulses coming from the encoder and rotate the
motor accordingly. This task will be accomplished by the firmware running on the
microcontroller (which is connected to the rotary encoder and the motor driver).

Ideally, there should be no delay, no jitter and the output should follow the 
*gear reduction formula* precisely. Since both the motor and the encoder have 
finite resolutions we can never match the ideal in terms of precision (though 
real gears are also not perfect). Time needed for responding to the input will 
also introduce a lag. We can also introduce [jitter](https://www.itprc.com/jitter-vs-latency/)
depending on the control mechanism we choose, as we will discuss.

## Gear reduction formula
Let's define the desired gear ratio as a rational number, **R**:
> R = N / D

Which means, for every D rotations of the input axis, the output axis should do
N rotations.

Every rotation of the input generates **E** encoder pulses and we need to generate
**S** steps for a revolution of the output.
> E * D = N * S

> E = R * S

For the lathe gearbox usage, the motor is connected to a lead screw that converts the
rotational movement to a linear one. The motion is defined by the pitch of the lead 
screw, **P<sub>L</sub>**, which defines the distance traveled per rotation.

The desired pitch, **P<sub>D</sub>**, is the pitch of the thread to we want to cut. Every
revolution of the spindle (input axis) should result in a movement of **P<sub>D</sub>**. 
The lead screw would need to make **P<sub>D</sub>** / **P<sub>L</sub>** turns to achieve this.
So, the ratio becomes:

> R = P<sub>D</sub> / P<sub>L</sub>

If we plug this in the third equation we get:
> E = P<sub>D</sub> * S / P<sub>L</sub>

Dividing this by E will give us the number of step pulses we need to generate for
each encoder pulse. Lets call this rational number, **R<sub>Pulse</sub>**.

> R<sub>Pulse</sub> = (P<sub>D</sub> * S) / (P<sub>L</sub> * E)

*Example:*

* Desired pitch   P<sub>D</sub> = 0.7 mm (for M6 thread).
* Leadscrew pitch P<sub>L</sub> = 2 mm.
* Encoder resolution   E = 2400 pulses per revolution.
* Steps per revolution S = 200 * 8 microsteps = 1600

    R<sub>Pulse</sub> = ((7/10) * 1600) / (2 * 2400)

    = 1120/4800

    = 7/30

There can be intermediate gears between the input and output. Pulse ratio can be
calculated by factoring in their ratios.

You can play with the pulse ratio calculator here in this online C++ compiler:

[Ratio calculator](https://wandbox.org/permlink/JTCG7lHs9tv3VubA)

>Code there is pretty much the same as the firmware. Using rational numbers,
provided by [boost library](https://www.boost.org/doc/libs/1_72_0/libs/rational/index.html), 
help calculate operations on them accurately and efficiently.
Thanks also to C++11 user defined literals, it is possible to define pitches very
intuitively. They are converted to rational numbers without any precision loss.

```cpp
auto my_pitch = 47.9_tpi;  // British size 6BA apparently
```

## Control algorithm
The first mechanism that comes to mind is to periodically (e.g. every millisecond)
measure the input and apply any corrections to the output to match the desired ratio. 

By selecting an appropriate cycle time, this method would be able to handle large
range of gear ratios including gear up (output rotates faster). It is also simple
to implement.

James Clough over at Clough42 Youtube channel had been working on his electronic lead 
screw project since last year. His implementation is incorporating this method and
he made great progress in demonstrating it's capability.
This particular below video goes into great detail about his design and I would 
recommend this series and his channel highly.

[![Video](https://img.youtube.com/vi/hLFzqSpVetE/0.jpg)](https://www.youtube.com/watch?v=hLFzqSpVetE)

There are some implications of this method which we can improve upon.
* Calculating the output position exactly requires 64 bit multiplication and division
  which takes a long time to execute. Switching to floating point arithmetic
  speeds it up (thanks to the hardware floating point unit) but introduces error.
  * In the video, James calculates the error as 117 micro steps for a typical thread
    pitch selection and argues that it would take a very long time to gradually reach
    that error amount. However an encoder overflow/underflow will cause the error 
    to change abruptly. This may possibly cause the motor to stall and lose synchronization. 
    One scenario this could show up is reversing the spindle rotation for a previous operation, 
    which can leave the overflow/underflow point in the middle of the thread to be cut.
* Sensitive to the cycle time.
   * In order to decrease response time, the cycle time needs to be small enough. But
   gear calculation time limits the minimum feasible.
   * Jitter. Output will have variable latency as the cycle time (i.e. sampling
   period) will not equally divide the encoder pulse period (except speeds corresponding
   to multiples of cycle frequency).

We will try to eliminate these the drawbacks.

### Incremental calculation
Looking at the **R<sub>Pulse</sub>** equation we can see that it is equivalent to the equation
of a 2D line. That allows us to use an algorithm called
[Bresenham](https://www.cs.helsinki.fi/group/goa/mallinnus/lines/bresenh.html) line drawing
algorithm.

In a nutshell, Bresenham incrementally calculates the division and remainder of
points along the line to decide whether this or next pixel up is the better choice. 
So, instead of periodically measuring the rotation delta and reevaluating the gear 
ratio expression at each cycle, we will respond to the encoder pulses as they happen 
and incrementally calculate the output using only integer addition and subtraction.

Below is a C++ implementation of this method:
```cpp
namespace gear {
  uint16_t enc_last_count = 0;
  int N = 7, D = 30; // Pulse ratio is 7/30
  int error = 0; // "sub-pixel" error
}

void InterruptHandler() { // Called when encoder value changes.
  using namespace gear;
  auto e = Encoder::get_count();
  auto step_dir = Dir::none;
  if (enc_last_count + 1 == e) { // forward
    error += N; // update error
    if ((error << 1) >= D) { // is the other 'pixel' closer?
      error -= D; // yes, trim the error
      step_dir = Dir::fwd;
    }
  }
  else { // backward
    error -= N;
    if ((err << 1) <= -D) {
      error += D;
      step_dir = Dir::back;
    }
  }
  if (step_dir != Dir::none) {
    step_gen::step(step_dir);
  }
  enc_last_count = e; // save
}
```
>Step generation is not shown but uses a separate timer peripheral in the 
microcontroller to create a precise (14 ns resolution) step pulse output. Pulse 
parameters are to be customized for the motor driver selected.

Advantage of this approach is that it does not rely on sampling at fixed intervals.
And floating point representation error is eliminated as it is integer operations only. 
It is also faster to calculate using only addition, substraction and shift operations
which are single cycle instructions.

One disadvantage is that the algorithm requires the pulse ratio to be less than 1.
This limits how large the desired pitch can be. Upper bound can be calculated by
plugging in your parameters to the pulse ratio equation and setting its value to 1.
For example with a E = 4000 PPR, S = 1600 SPS (200 steps/rev, 8 microstepping) and a lead screw 
pitch of 12 TPI, desired pitch should be shorter than 4.8 TPI (~5.3 mm pitch). 
That is quite a large pitch! So, for the lathe gearbox use case this is not 
a significant limitation. Note that the output axis would be rotating close to 2.5x the
speed of the input at this point. Increasing the encoder resolution or decreasing 
the SPS would increase the maximum ratio. On most lathe setups, the encoder would
need to be connected via a belt or gear system because it can not be in axis. So
a smaller pulley on the encoder would have the effect of increasing the resolution
(pulses/turn). Speed ratings of the encoder should be observed, of course.

>For use cases where even a higher ratio is desired there are techniques we that can be used.
One example is to generate intermediate step pulses between Bresenham steps. We
can space them in time accurately (within the limits of the timer resolution) if we keep
track of the input speed. There are motor controllers that have micro-step interpolation
mechanisms that achieve a similar goal

With this approach, I was able to get a total latency around 1.2 microseconds vs
6+ reported for the ELS project despite using a lower speed microcontroller.

## Latency
Although these calculations are faster and lack floating point error, there is an 
unavoidable latency coming from the [interrupt processing mechanism](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka16366.html)
and the latency of flushing flash memory buffer. I measured the total latency in 
servicing the pin change interrupt and toggling an output pin to be around 0.7 microseconds.
> Some microcontrollers have a small additional RAM section that can be used for
avoiding flash memory overhead for interrupt handling.

In order to improve the latency even further we will change our algorithm to make 
better use of the microcontroller hardware.

### Two pointers
Instead of updating the error and deciding whether to make a step, at every encoder 
pulse, we can calculate how many of them are required to generate a step pulse.

We can determine that value, **k**, by transforming the Bresenham equation to an inequality
and finding the boundary value.

> 2 * (Error + k * N) >= D

Solving for smallest value of k, and adding a proper offset to the dividend to 
get a correct integer division result, we get:

> k = (D - 2 * Error + 2 * N - 1) / (2 * N)

> Error = Error + k * N - D (after k encoder pulses)

where D and N are the numerator and denominator of the pulse ratio, **R<sub>Pulse</sub>**. And
k is the number of encoder pulses needed to make the next step. This is for one
direction. There is a similar equation for k value of the opposite direction.

> k_reverse = 1 + ((D + 2 * Error) / (2 * N));

> These compile to more expensive instructions (like divide) compared to the 
Bresenham but it does not require 64 bit integers. 32 bit integer division only takes 12 cycles or
less. So both terms can be calculated quite quickly. Even better, this time will not
appear in the response latency.

![bresenham](https://user-images.githubusercontent.com/61201064/75373569-68c42f80-587f-11ea-968e-1c06bf8d11f5.png)

For the image above, consider the horizontal axis as the encoder position and the
vertical axis as the step pulses generated. The slope is **R<sub>Pulse</sub>**. 
It shows a finite line but in our application the line can be considered infinite.
Here there are 4 steps taken and k values for each interval are 2, 2, 3, 2, 2.

Every point on the line is inside an interval where no steps are generated. When the
encoder value goes outside the interval we need to update to the new error and
calculate the new region in addition to generating step signals. We can use the
capabilities of the microcontroller a very low latency trigger for the step pulse timer. 

### Peripheral action
[Timer peripherals](https://www.st.com/content/ccc/resource/technical/document/application_note/group0/91/01/84/3f/7c/67/41/3f/DM00236305/files/DM00236305.pdf/jcr:content/translations/en.DM00236305.pdf)(pdf)
 on the STM32F103 microcontroller have 4 channels. For the encoder
timer (Timer 1), the encoder is connected to two of the channels as input and the
timer is set to encoder mode. We can still use the remaining two channels. We will
use them for marking the encoder values for the next step pulse. The encoder value 
will either keep moving in the same direction (most of the time) or it may change
direction.

So we will use one of the remaining channels (Channel 3) for the encoder value that
a new step will be made (same direction), the other channel for when it needs 
to change direction. When the encoder value matches one of the channels, it can 
generate a trigger output, trigger an interrupt, etc. For channel 3, we enable 
the trigger output and interrupt. Channel 4 will only trigger an interrupt where 
we will do the direction change and manually generate a step pulse.

For example, consider the image above and let the encoder value be 5. The two step
points are at encoder values 7 and 3. So one channel is at 7 and the other at 3. 
We calculated these when we made the previous step, either at encoder position 4
or position 6 depending on which direction the input was rotating.

Step pulse generation timer (Timer 3) will be configured to be triggered by the 
trigger output of Timer 1. This allows Timer 1 to directly trigger Timer 3 without
CPU intervention and start the pulse generation within a couple of clock cycles. The 
interrupt routine will be serviced after, to calculate the next region thus will
not cause any latency in the step pulse generation.

Below is a capture of the encoder and step signals zoomed in to a step pulse generation:
![low_latency](https://user-images.githubusercontent.com/61201064/75390684-8902e700-589d-11ea-825a-9808be902c12.png)

Total latency, the time difference between getting switching of the encoder signal
and start of the step pulse shows a ~41 nanoseconds (measured with a 24 Mhz
logic analyzer which has 41.6 ns resolution :p). An oscilloscope would be more
accurate but let's not split hairs. That is 30x the response speed of our previous
attempt.

## Out'a Phase
Before patting ourselves in the back for a job well done, let's look at the zoomed
out view.

![big_picture](https://user-images.githubusercontent.com/61201064/75391567-31fe1180-589f-11ea-81f3-287e54d778dd.png)

While the encoder pulses are coming along nicely, our steps pulses (which are 2 
microseconds long so they look like spikes at this time scale) are not uniformly
spaced. This means that we are asking the motor to constantly change speed, even
if the average is correct. How come?

>The capture was done with a pulse ratio of 2/3 which corresponds only having two
intervals with lengths of 1 and 2. This makes it easier to observe the irregularity.

This looks like the same jaggedness we would see if we drew a line with a slope
of 2/3 on a black and white display. Bresenham algorithm calculates this error 
value which is very similar to a remainder in a division. For the Bresenham case 
it is proportional to how much the selected pixel is away from the actual line. 
If we add in the denominator that had been eliminated for more efficient calculation, 
we get the error ratio:

> R<sub>error</sub> = Error / D

The value of which is in the range [0, 0.5). It shows how much of a *pixel* we are
off. For our case, every time we generate a step pulse, we are either just on 
time (Error = 0), or we are *a bit* early, proportional to the error ratio.

We can define the pixel as the time between step pulses, T<sub>step</sub>. Now
we can calculate how early we are by simply multiplying it with the error ratio:

> T<sub>error</sub> = T<sub>step</sub> * R<sub>error</sub>

So we can compensate the time error, T<sub>error</sub>, by delaying the step 
pulse by that amount. But in order to calculate it, we need to calculate the
pulse period, T<sub>step</sub>.

Gear ratio is valid for the position, but also for speed. So if we can measure
the time between encoder signals, T<sub>enc</sub>, we can calculate T<sub>step</sub>
by dividing that with the pulse ratio.

> T<sub>step</sub> = T<sub>enc</sub> / R<sub>pulse</sub>

Writing the error time in terms of encoder time, the **D** will cancel out and we get:

> T<sub>error</sub> = T<sub>enc</sub> * Error / N

### Phase shift
Now we just need to measure the time between encoder events, and we can use that
to calculate T<sub>error</sub> the delay we are going to add for the next step 
pulse (as an offset to Timer 3), to eliminate the phase error.

We can setup the pin change interrupt and sample another timer in counter mode to
determine the time between encoder pulses. It is not ideal as interrupt services 
has variable delay but should work.

A better performing approach is to use another timer in *PWM Input Mode* and tie one of the 
inputs to one of the encoder channels. Period measured will be 4x of the period of 
the encoder changes since we are using only one channel and measuring a whole cycle. 
Setting the prescaler to 4 automatically handles the division and increases maximum measurable 
duration while reducing the resolution to 72 ns. Using DMA to read the capture values 
means there is no CPU overhead. On the downside, we will be using one more timer.

> I would like to have the timer peripheral to have a "counter change" event which
can trigger a DMA request (not sure if any STM32 chip has that feature). Some 
microcontrollers, like STM32F3 series, have timers that have 6 channels. With those
we could use the two additional channels like a counter change event, by keeping them 1 count away from the current counter, similar to the two pointers 
approach for the step generation. Channels can trigger DMA so this would also allow 
low jitter measurement of elapsed time without decreasing the measurement resolution. In
any case we are using 3 timers for this phase compensated version.

After putting it all together, here is the new pulse train:
![all_good](https://user-images.githubusercontent.com/61201064/75399576-74c8e500-58b1-11ea-963f-ad68c0a2afd4.png)

We can see that the pulses are now uniformly spaced. Speed is maintained. The last 
signal, tim1c3, is the trigger signal from the encoder timer. It shows what our step
signal was before the phase compensation.

Now we are responding with both very low latency and jitter, generating a smooth
and accurate step signal.

Below is a snapshot showing step pulses being generated at 140+ Khz, which would 
correspond to 5250+ RPM on the stepper motor (motor was disconnected during this 
test as stepper motors have limited maximum speed).

![high speed](https://user-images.githubusercontent.com/61201064/81491014-b37a0480-923d-11ea-98c0-879f67c89617.png)


## Changing gears
Switching to a new gear ratio (i.e. thread pitch) simply entails updating the
pulse ratio, expressed as two integers N and D, with the new value. Error should
also be reset to 0.

One thing to note is that switching to a different ratio may require a significant 
change in motor speed (i.e. a short pitch to long pitch selection). The motor should
be accelerated/decelerated to the new speed before switching to the gear mechanism discussed above.
