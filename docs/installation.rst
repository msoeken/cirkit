Installation
============

Requirements and dependencies
-----------------------------

The following software is required in order to build CirKit

* git
* cmake (at least version 3.0.0)
* g++ (at least version 4.9.0) or clang++ (at least version 3.5.0)
* boost (at least version 1.56.0)
* GNU MP, and its C++ interface GMP++
* GNU readline

Installing dependencies in Ubuntu Linux
```````````````````````````````````````

In *Ubuntu* the packages can be installed with::

  sudo apt-get install build-essential git g++ cmake libboost-all-dev libgmp3-dev libxml2-dev zlib1g-dev lapack openblas

Installing dependencies in Arch Linux
`````````````````````````````````````

In *arch* the packages can be installed with::

  sudo pacman -S base-devel git g++ cmake boost boost-libs gmp libxml2 zlib lapack

Installing dependencies in Mac OS
`````````````````````````````````

In *Mac* it's recommended to use Homebrew_ to install the required packages::

  brew install boost cmake gmp readline git lapack openblas

.. _Homebrew: http://brew.sh/

Build and run CirKit
--------------------

After extracting or cloning CirKit perform the following steps::

  mkdir build
  cd build
  cmake ..
  make external
  make cirkit

CirKit can be executed with::

  build/programs/cirkit

Build and Run RevKit
--------------------

After extracting or cloning CirKit perform the following steps::

  mkdir build
  cd build
  cmake -Denable_cirkit-addon-reversible=ON -Denable_cirkit-addon-formal=ON ..
  make external
  make revkit

RevKit can be executed with::

  build/programs/revkit

Troubleshooting
---------------

No recent cmake version
```````````````````````

If for some reason only older versions of ``cmake`` are available on
the system, you can install cmake using ``utils/tools.py install
cmake`` directly from CirKit. Afterwards update the ``$PATH`` variable
by typing ``export PATH=<path-to-cirkit>/ext/bin:$PATH``.

Libraries not found
```````````````````

It's best to set the environment variable ``CIRKIT_HOME`` to the
directory of CirKit using::

  export CIRKIT_HOME=<full-path-to-cirkit>

or some other command depending on the user's shell.  Also, sometimes
depending libraries are not found, then run::

  export LD_LIBRARY_PATH=$CIRKIT_HOME/ext/lib:$LD_LIBRARY_PATH

in Linux or::

  export DYLD_LIBRARY_PATH=$CIRKIT_HOME/ext/lib:$DYLD_LIBRARY_PATH

in Mac OS.

No recent Boost version
```````````````````````

There is a manual way to install Boost and CirKit's package manager
can help.  Before installing CirKit run::

  mkdir build
  ./utils/tools.py install boost
  cd build
  cmake -DBoost_NO_SYSTEM_PATHS=TRUE -DBOOST_ROOT:PATHNAME=`pwd`/tools/boost_1_63_0/ ..
  make external
  make cirkit

Of course, one can add further options to the ``cmake`` command in the
fourth line, e.g., to build RevKit.
