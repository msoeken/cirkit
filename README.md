# CirKit

## Requirements

The following software is required in order to build CirKit

* git
* cmake (at least version 2.8.9)
* g++ (at least version 4.9.0) or clang++ (at least version 3.5.0)
* boost (at least version 1.48.0)
* GNU MP, and its C++ interface GMP++
* libxml2

## Build CirKit

After cloning CirKit perform the following steps

    mkdir build
    cd build
    cmake ..
    make

## Build CirKit Addons

The easiest way to enable addons is by typing `ccmake ..` in the build directory

## Executing CirKit Programs

Executables can be found in the directory `build/programs`. From the build
directory call e.g. `programs/bdd_info`

## Package Manager

CirKit uses some external (mainly academic) programs that are typically not
shipped with Linux distributions.  To ease their installation CirKit provides
its own small package manager, that can be invoked via `utils/tools.py`.  Run

    utils/tools.py

to learn how it can be executed.  The programs are automatically downloaded and
build, binaries are installed in `ext/bin`.
