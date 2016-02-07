# RGB_DMD LED Matrix Display for Pinball #

RGB_DMD is software for displaying either colorized signals from a pinball machine or color images from a host PC onto RGB LED matrix displays using the Teensy 3.2 development board and a custom carrier board. RGB_DMD was written by Eli Curtz, <eli@nuprometheus.com> but is built on top of open source work by many people, primarily the [SmartMatrix](http://docs.pixelmatix.com/SmartMatrix/library.html) display library.

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
[SmartMatrix](https://github.com/ecurtz/SmartMatrix) is used for outputting the signal to the RGB matrix. Link is to my fork which is required for hardware compatibility and has some optimizations.

[FastCRC](https://github.com/FrankBoesing/FastCRC) is used to calculate a checksum for DMD frames so they can be uniquely identified.

### Settings ###
RGB_DMD should be built to run at the __96MHz optimized (overclock)__ setting using the __Tools : CPU Speed__ menu in the Arduino IDE.