# RevKit 2.0 Base System

## Requirements

The following software is required in order to build RevKit

* git
* cmake (at least version 2.8.9)
* g++ or clang++ (with C++11 support)
* boost (at least version 1.48.0)
* GNU MP, and its C++ interface GMP++
* Qt5 (only for the GUI)

## Build RevKit

After cloning RevKit perform the following steps

    mkdir build
    cd build
    cmake ..
    make
    
To build the GUI perform the following steps from the base directory

    cd gui
    mkdir build
    cd build
    cmake ..
    make

## Addons

The RevKit base system contains basic data structures and several functions for
reversible circuit design.  Further, many algorithms for optimization,
simulation, and synthesis are implemented.  More advanced functionality can be
added via addons.  Please take a look into the
[wiki](https://github.com/msoeken/revkit/wiki) and the addons directory for
further information.

## About

This system is maintained by Mathias Soeken (msoeken@cs.uni-bremen.de). Parts
have been contributed by Nabila Abdessaied, Stefan Frehse, Aaron Lye, Nils
Przigoda, Laura Tague, and Robert Wille.

RevKit 2.0 has undergone some heavy modifications and therefore some functions
that are contained in [RevKit 1.3](http://www.revkit.org) are not available
yet (e.g. quantum gates). For quantum gates an distinguished addon will soon be
provided.  The GUI is currently ported from Python to C++.
