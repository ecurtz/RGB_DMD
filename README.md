# RGB_DMD LED Matrix Display for Pinball #

RGB_DMD is software for displaying either colorized signals from a pinball machine or color images from a host PC onto RGB LED matrix displays using the [Teensy 3.2](http://www.pjrc.com/teensy/index.html) development board and a custom carrier board. RGB_DMD was written by Eli Curtz, <eli@nuprometheus.com> but is built on top of open source work by many people, primarily the [SmartMatrix](http://docs.pixelmatix.com/SmartMatrix/library.html) display library.

## Contents ##
### License ###
The RGB_DMD source code is released under the MIT License.
### HEX Folder ###
Contains compiled .hex versions of the project, which can be loaded directly to the board using the Teensy Loader  or Teensy Qt applications.
### SRC Folder ###
Contains the Arduino / C++ source code for the project.
### UTIL Folder ###
Utilities for testing and configuring data.

## Building RGB_DMD ##

### Tools ###
[Arduino IDE](http://arduino.cc/en/main/software) is required for recompiling RGB_DMD from source or customizing the code.

[Teensyduino](http://www.pjrc.com/teensy/td_download.html) is a required plugin that adds Teensy support to the Arduino IDE as well as the Teensy Loader tool for transferring compiled code to the development board.

### Libraries
These are additional third party code required when building RGB_DMD. They are distributed as source on github and are installed as described under __Manual Installation__ at the [Arduino reference](https://www.arduino.cc/en/Guide/Libraries#toc5).

[SmartMatrix](https://github.com/ecurtz/SmartMatrix) is used for outputting the signal to the RGB matrix. Link is to my fork which is required for hardware compatibility and has some optimizations.

[FastCRC](https://github.com/FrankBoesing/FastCRC) is used to calculate a checksum for DMD frames so they can be uniquely identified.

### Settings ###
The Arduino IDE should be set to compile for Teensy by selecting __Teensy 3.1__ from the __Tools : Board__ menu in the Arduino IDE.

RGB_DMD should be built to run at the __96MHz optimized (overclock)__ setting using the __Tools : CPU Speed__ menu.

## Using DMD Input ##
Pinball machines use a set of 6 signals to control a plasma or LED DMD. RGB_DMD captures these inputs to determine which CPU system the pin is using and rebuild the display images.

__DOT CLOCK__: Repeatedly switches between high and low to mark when the dot data should be read.
__DOT DATA__: Will be either on or off for each of the 128 dots in a single row of the DMD.
__COLUMN LATCH__: Marks the end of the row of dot data.
__DISPLAY ENABLE__: Turns the actual display output on and off. By varying the display enable time the overall brightness can be adjusted, and varying the time for different data on the same row is used to produce the effect of multiple shades on a single color DMD.

__ROW CLOCK__: Moves to the next display row. Separate from the column latch because one row may be updated and latched several times with different data before the transition.__ROW DATA__: Used with row clock to mark the transition from the last row back to the first.

## Using USB Input ##
RGB_DMD may be controlled over USB from a host computer to store settings for the DMD input or display full color images for homebrew pinball machines.
### Sending From Python ###
Using USB input from Python requires the [PySerial Extension](https://pyserial.readthedocs.org/en/latest/). If you are using [PyProcGame](http://pyprocgame.pindev.org) or the [Mission Pinball Framework](https://missionpinball.com) PySerial may already be installed.
### Sending From C/C++/C# ###