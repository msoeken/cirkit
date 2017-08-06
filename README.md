# CirKit

CirKit is a software library and framework for logic synthesis.

## Requirements

The following software is required in order to build CirKit

* git
* cmake (at least version 3.0.0)

  *If for some reason only older versions are available on the system, you can install cmake using `utils/tools.py install cmake` directly from CirKit. Afterwards update the `$PATH` variable by typing `export PATH=<path-to-cirkit>/ext/bin:$PATH`.*
* g++ (at least version 4.9.0) or clang++ (at least version 3.5.0)
* boost (at least version 1.56.0)
* GNU MP, and its C++ interface GMP++
* GNU readline

In *Ubuntu* the packages can be installed with

    sudo apt-get install build-essential git g++ cmake libboost-all-dev libgmp3-dev libxml2-dev zlib1g-dev lapack openblas

In *arch* the packages can be installed with

    sudo pacman -S base-devel git g++ cmake boost boost-libs gmp libxml2 zlib lapack

In *Mac* it's recommended to use [Homebrew](http://brew.sh/) to install the required packages:

    brew install boost cmake gmp readline git lapack openblas

## Build and Run CirKit

After extracting or cloning CirKit perform the following steps

    mkdir build
    cd build
    cmake ..
    make external
    make cirkit

CirKit can be executed with

    build/programs/cirkit

Check the [documentation](http://msoeken.github.io/cirkit_doc.html) for more details.

## Build and Run RevKit

After extracting or cloning CirKit perform the following steps

    mkdir build
    cd build
    cmake -Denable_cirkit-addon-reversible=ON -Denable_cirkit-addon-formal=ON ..
    make external
    make revkit

RevKit can be executed with

    build/programs/revkit

Check the [documentation](http://msoeken.github.io/cirkit_doc.html) for more details.

## Troubleshooting

### Libraries not found

It's best to set the environment variable `CIRKIT_HOME` to the
directory of CirKit using

    export CIRKIT_HOME=<full-path-to-cirkit>

or some other command depending on the user's shell.  Also, sometimes
depending libraries are not found, then run

    export LD_LIBRARY_PATH=$CIRKIT_HOME/ext/lib:$LD_LIBRARY_PATH

on Linux or

    export DYLD_LIBRARY_PATH=$CIRKIT_HOME/ext/lib:$DYLD_LIBRARY_PATH

on Mac OS.

### No recent Boost version

There is a manual way to install Boost and CirKit's package manager
can help.  Before installing CirKit run:

    mkdir build
    ./utils/tools.py install boost
    cd build
    cmake -DBoost_NO_SYSTEM_PATHS=TRUE -DBOOST_ROOT:PATHNAME=`pwd`/tools/boost_1_63_0/ ..
    make external
    make cirkit

Of course, one can add further options to the `cmake` command in the
fourth line, e.g., to build RevKit.

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
its own small package manager that can be invoked via `utils/tools.py`.  Run

    utils/tools.py commands

to learn how it can be executed.  The programs are automatically downloaded and
build, binaries are installed in `ext/bin`.
