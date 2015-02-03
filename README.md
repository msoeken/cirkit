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

After extracting or cloning CirKit perform the following steps

    mkdir build
    cd build
    cmake ..
    make

## Executing CirKit Programs

Executables can be found in the directory `build/programs`. From the build
directory call e.g. `programs/bdd_info`

## Build CirKit Addons

CirKit can be extended by addons, you can learn more in the `addons/README.md`
file.  The addons are not included into the build process by default.  The
easiest way to enable addons is by typing `ccmake ..` in the build directory
which opens the ncurses GUI of cmake.  An addon can be enabled by toggling the
flag at the entry `enable_cirkit-addon-*`.  Afterwards, press `c` followed by
`g` and then recompile with `make`.

## Package Manager

CirKit uses some external (mainly academic) programs that are typically not
shipped with Linux distributions.  To ease their installation CirKit provides
its own small package manager, that can be invoked via `utils/tools.py`.  Run

    utils/tools.py commands

to learn how it can be executed.  The programs are automatically downloaded and
build, binaries are installed in `ext/bin`.
