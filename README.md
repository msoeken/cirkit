[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Documentation Status](https://readthedocs.org/projects/cirkit/badge/?version=latest)](http://cirkit.readthedocs.io/en/latest/?badge=latest)

# CirKit

CirKit is a software library and framework for logic synthesis.

## Quick installation guide

This is the quick installation guide for CirKit and RevKit assuming that [all requirements](https://cirkit.readthedocs.io/en/latest/installation.html#requirements-and-dependencies) are met.

### CirKit

Use git to build CirKit with the following steps

    git clone --recursive https://github.com/msoeken/cirkit.git
    mkdir build
    cd build
    cmake ..
    make cirkit

CirKit can be executed with

    build/programs/cirkit

### RevKit

Use git to build RevKit with the following steps

    git clone --recursive https://github.com/msoeken/cirkit.git
    mkdir build
    cd build
    cmake -Denable_cirkit-addon-reversible=ON -Denable_cirkit-addon-formal=ON ..
    make revkit

RevKit can be executed with

    build/programs/revkit

## Detailed installation and documentation

The documentation can be found at [cirkit.readthedocs.io](https://cirkit.readthedocs.io/en/latest).
