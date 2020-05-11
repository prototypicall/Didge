# Didge project - STM32 "Blue pill" lathe digital gearbox firmware
Makefile based bare-metal C++ project.

## Environment
Linux-like command line environment with Makefile build support. I had good
success with [Msys2](https://www.msys2.org/), which comes with a terminal and a 
package manager, on a Windows machine.

*TODO*: Environment setup instructions

## Toolchain
[GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
(Based on GCC 9.2 at this time, frequently updated).

Comes with a couple of C/C++ examples with make files, which this build setup is based on.

## Programming
We need to upload generated binary to the microcontroller. There are various
methods like using the microcontroller built-in USB/Serial upload capabilities or
using a programming devices like [ST-Link](https://www.st.com/en/development-tools/st-link-v2.html), 
[J-Link](https://www.segger.com/products/debug-probes/j-link/), etc. There are low
cost ST-Link V2 compatible USB devices. [OpenOCD](http://openocd.org/documentation/) provides 
programming and debugging server, available on many platforms and supports many
devices, including ST-Link. It's also available as a [package in Msys2](https://packages.msys2.org/base/mingw-w64-openocd).

## Dependencies
* [Boost C++ Libraries](https://www.boost.org/): Using only the [Rational]((https://www.boost.org/doc/libs/1_72_0/libs/rational/index.html)
library. Note that this is one of the *header only* libraries so you don't need to
compile or install the Boost library. You just need to add its location as an include path in the Makefile.

I have a modified implementation which has exception throwing code removed for 
saving around 1kB of binary size. Modified version can be found [here]. 
It can be copied over the same location as the original file.

* [Kvasir](http://kvasir.io/) Library: For configuring the registers of the microcontroller
in a more reasonable way. In a nutshell, it is composed of a flexible bit-field aware
memory access mechanism and an auto generated register definition source files (generated
from vendor supplied [CMSIS-SVD](https://arm-software.github.io/CMSIS_5/SVD/html/svd_Format_pg.html) XML files.
This allows to write more readable register access code and leaves the bug prone bit-manipulation 
part to the compiler. Unfortunately the project is not active and lacks a good documentation.
> ST Micro provides a GUI code generator [STM32CubeMx](https://www.st.com/en/development-tools/stm32cubemx.html)
which can generate a complete project (some IDEs and also makefile). It can be very useful 
for evaluation and basic use cases. The abstraction layer has noticeable overhead, though. 
For example, an empty project (setting up clock and setting hardware registers to their 
initial values) would compile to a 6 KB binary where as prototype version of this firmware with all
the gear emulation part would only be a 3 KB binary, the gap would increase as more
and more HAL API is used. Some runtime checks within API functions are not actuallu
not required for constant (compile time calculate-able) values. For advanced use
of the peripherals, the GUI for configuration was not very useful. One thing that
it is very useful is figuring out the pin configuration of a project. 

Register definition files are flattened (coming from the SVD files) so it is not 
very useful for cases like enumerated values for bit fields or any groups (e.g. all channels
of the DMA have the same register layout but they are accessible only explicitly). 
There are also some minor errors and missing registers in the SVD files. I implemented 
a couple of changes and additions for the STM32F103x tree which allows me to access 
pins and interrupts more generically (i.e. via indices).
