# CirKit and RevKit (version 3)

CirKit and RevKit are synthesis and optimization frameworks for classical and
quantum logic synthesis, respectively.  They are implemented based on various
[EPFL logic sythesis libraries](https://github.com/lsils/lstools-showcase).

## Installation (shell interface)

```bash
# CirKit
mkdir build
cd build
cmake ..
make cirkit
cli/cirkit

# RevKit
mkdir build
cd build
cmake ..
make revkit
cli/revkit
```

## Installation (Python library)

```bash
# CirKit
cd dist/cirkit
python3 setup.py install

# RevKit
cd dist/revkit
python3 setup.py install
```

## With custom compiler

CirKit and RevKit are implemented using C++-17 features and therefore a recent
compiler is required (e.g., GCC ≥ 7.3.0 and Clang ≥ 6.0.0).  If your recent
compiler is not on the `PATH` prefix the `make` and `python3` commands as
follows:

```bash
CXX=/path/to/c++-compiler make cirkit

CC=/path/to/c++-compiler python3 setup.py install
```

## CirKit and RevKit 2.0

The 2.0 versions of CirKit and RevKit can be found
in the [develop](https://github.com/msoeken/cirkit/tree/develop/) branch.


## EPFL logic sythesis libraries

CirKit and Revkit are based on the [EPFL logic synthesis](https://lsi.epfl.ch/page-138455-en.html) libraries.  The libraries and several examples on how to use and integrate the libraries can be found in the [logic synthesis tool showcase](https://github.com/lsils/lstools-showcase).


