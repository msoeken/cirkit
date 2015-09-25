# CirKit 2.0 Addons

## Installation

Add a new addon by cloning the respective git repository into this folder. Afwards perform the following steps in the `build' directory

    cmake ..
    make

The directory name of the addon must start with *revkit-addon-*

## Usage

Put all library source code into *src/core* and *src/algorithms*. Executable programs require one C++ source file in the *programs* directory and are found in the *build/programs* directory after successful compilation.


