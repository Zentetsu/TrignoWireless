# Overview

TrignoWireless is a sublibrary for LSL. It provides all the information from the EMG in four streams (EMG, ACC, IMEMG and AUX)

# Usage

First launch Trigno Control Utility, wait until the software is connected to the box. Turn on all the EMGs you want to use.
When the application is launched, you can choose the channel you want to extract from the EMG. Then, enter the ID for each EMG you want to record. 
To finshed just click on "Connect" to link the application to the TrignoWireless software.

# Build instructions

## Build LSL

To start, you must install the LSL library by following each step mentionned on this link: [LSL build](https://github.com/sccn/labstreaminglayer/blob/master/doc/BUILD.md).

## Build TrignoWireless library

Before building this library, be sure you have installed the Trigno Control Utility and that the firmware of the Trigno box and  EMGs are up to date. You can find it here: [Delsys Software](https://www.delsys.com/support/software/).

### Windows users

Step to follow:<BR/>
* Create the "build" foledr at the root
* Open a terminal and move into this one
* Run this command line :  `build> cmake .. -G "Visual Studio 16 2019" -DQt5_DIR="C:\Qt\version\msvc_version\cmake\Qt5" -DBOOST_DIR="C:\local\boost_version"`
* Run this command line :  `build> cmake --build . --config Release --target install`

### Linux and OSX users

Step to follow:<BR/>
* Open a terminal at the root of the library
* Run this command line :  `TW> mkdir build & cd build`
* Run this command line :  `TW> ccmake ..`
* Press "c" to configure
* Fill the path for the LSL library, Qt5 and Boost if there're missing and press c again
* Press "q" to quit
* Run this command line :  `TW> cmake --build . --target install`