#!/usr/bin/env python

# CirKit: A circuit toolkit
# Copyright (C) 2009-2015  University of Bremen
# Copyright (C) 2015-2017  EPFL
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


################################################################################
# MINIMALISTIC PACKAGE MANAGER                                                 #
################################################################################
#                                                                              #
# Author : Mathias Soeken (2014)                                               #
#                                                                              #
# This is a very small package manager that helps to download and install some #
# often used executables which are no necessary requirement for building       #
# cirkit. Hence, they are not integrated into the cmake build flow to keep     #
# that one as simple as possible.  Also the flow is not suitable for           #
# installing libraries.                                                        #
#                                                                              #
# New packages can be added in the Packages section.  The flow works for the   #
# most common scenarios. A new scenario usually only requires the addition of  #
# a new fmt entry together with its handling in the checkout_or_download       #
# function                                                                     #
#                                                                              #
# Patches are automatically applied if they are provided as file in the        #
# directory utils/patches/<package_name>/*.patch (in case this script is used  #
# in a different context than cirkit, the patches directory must be in the     #
# folder as this file.                                                         #
#                                                                              #
################################################################################

import glob
import importlib
import inspect
import multiprocessing
import os
import shutil
import sys

################################################################################
# Packages                                                                     #
################################################################################

# Howto describe packages
#
# description - A short description for the `list' command
# subdir      - The subdir where the package will be maintained in build/tools.
#               This must coincide with directories that may be created by
#               checking out the source or extracting an archive
# url         - Either a string or a list of where to find the sources
# fmt         - Describes how the sources are obtained and extracted
#               hg        : Checkout with mercurial
#               tar-gz    : Downloads url and uses tar xfz for unpacking
#               wget-list : url must be a list, all files are downloaded and
#                           placed into subdir
# build       - A list of commands that should be executed for building
# install     - A list of commands that should be executed for installing
#               One can place a "%s" into a command to refer to the ext/bin
#               directory
# makedir     - Flag to enable the creation of directories (set it to
#               'True' if the archive is packed whithout directories)

class package_abc:
    description = "ABC: System for Sequential Logic Synthesis and Formal Verification"
    subdir      = "abc"
    url         = "https://bitbucket.org/alanmi/abc"
    fmt         = "hg"
    build       = [ "make -j%d"  % multiprocessing.cpu_count() ]
    install     = [ "cp -v abc %s" ]

class package_minisat:
    description = "Minimalistic, open-source SAT solver"
    subdir      = "minisat"
    url         = "http://minisat.se/downloads/minisat-2.2.0.tar.gz"
    fmt         = "tar-gz"
    build       = [ "MROOT=`pwd` make -C core rs" ]
    install     = [ "cp -v core/minisat_static %s/minisat" ]

class package_picosat:
    description = "PicoSAT SAT solver"
    subdir      = "picosat-959"
    url         = "http://fmv.jku.at/picosat/picosat-959.tar.gz"
    fmt         = "tar-gz"
    build       = [ "./configure", "make" ]
    install     = [ "cp -v picogcnf picomcs picomus picosat %s" ]

class package_sat13:
    description = "Knuth's CDCL SAT solver"
    subdir      = "sat13"
    url         = [ "http://www-cs-faculty.stanford.edu/~uno/programs/sat13.w", "http://tug.ctan.org/support/graphbase/gb_flip.w", "http://tug.ctan.org/support/graphbase/boilerplate.w" ]
    fmt         = "wget-list"
    build       = [ "ctangle gb_flip.w", "ctangle sat13.w", "gcc -o sat13 gb_flip.c sat13.c" ]
    install     = [ "cp -v sat13 %s" ]

class package_dimacs2sat:
    description = "Convert DIMACS to SAT for Knuth's SAT solvers"
    subdir      = "dimacs2sat"
    url         = [ "http://www-cs-faculty.stanford.edu/~uno/programs/dimacs-to-sat.w" ]
    fmt         = "wget-list"
    build       = [ "ctangle dimacs-to-sat.w", "gcc -o dimacs-to-sat dimacs-to-sat.c" ]
    install     = [ "cp -v dimacs-to-sat %s" ]

class package_espresso:
    description = "Espresso is a multi-valued PLA minimization (adjusted version)"
    subdir      = "espresso-ab-1.0"
    url         = "https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/eqntott/espresso-ab-1.0.tar.gz"
    fmt         = "tar-gz"
    build       = [ "./configure", "make CFLAGS=-ansi" ]
    install     = [ "cp -v src/espresso %s" ]

class package_directed_lad:
    description = "LAD is a program (in C) for solving the subgraph isomorphism problem (for directed and labelled graphs)"
    subdir      = "directedLAD"
    url         = "http://liris.cnrs.fr/csolnon/directedLAD.tgz"
    fmt         = "tgz"
    build       = [ "make" ]
    install     = [ "cp -v main %s/directed_lad" ]

class package_lad:
    description = "LAD is a program (in C) for solving the subgraph isomorphism problem"
    subdir      = "LAD"
    url         = "http://liris.cnrs.fr/csolnon/LAD.tgz"
    fmt         = "tgz"
    build       = [ "make" ]
    install     = [ "cp -v main %s/lad" ]

class package_bdd14:
    description = "Knuth's BDD package"
    subdir      = "bdd14"
    url         = [ "http://www-cs-faculty.stanford.edu/~uno/programs/bdd14.w", "http://tug.ctan.org/support/graphbase/gb_flip.w", "http://tug.ctan.org/support/graphbase/boilerplate.w" ]
    fmt         = "wget-list"
    build       = [ "ctangle gb_flip.w", "ctangle bdd14.w", "gcc -o bdd14 gb_flip.c bdd14.c" ]
    install     = [ "cp -v bdd14 %s" ]

class package_cmake:
    description = "A cross-platform, open-source build system"
    subdir      = "cmake-3.0.2"
    url         = "http://www.cmake.org/files/v3.0/cmake-3.0.2.tar.gz"
    fmt         = "tar-gz"
    build       = [ "./bootstrap --prefix=../../../ext", "make -j%d" % multiprocessing.cpu_count() ]
    install     = [ "make install" ]

class package_boost:
    description = "Boost C++ libraries"
    subdir      = "boost_1_63_0"
    url         = "http://downloads.sourceforge.net/project/boost/boost/1.63.0/boost_1_63_0.tar.gz"
    fmt         = "tar-gz"
    build       = [ "./bootstrap.sh --with-libraries=test,filesystem,graph,program_options,system,timer", "./b2 -j%d" % multiprocessing.cpu_count() ]
    install     = []

class package_yosys:
    description = "Yosys Open SYnthesis Suite"
    subdir      = "yosys"
    url         = "https://github.com/cliffordwolf/yosys"
    fmt         = "git"
    build       = [ "make config-clang", "make -j%d" % multiprocessing.cpu_count(), "make lib" ]
    install     = [ "for i in `find backends frontends kernel libs passes -type f \( -name '*.h' -o -name '*.hh' \) -printf '%h\n' | sort -u`; do mkdir -p ../../../ext/include/yosys/$i; done", "for i in `find backends frontends kernel libs passes -type f \( -name '*.h' -o -name '*.hh' \)`; do cp -v $i ../../../ext/include/yosys/$i; done", "cp -v yosys %s", "cp -v libyosys.so ../../../ext/lib" ]

class package_yices:
    description = "Yices SMT Solver"
    subdir      = "yices-2.4.1"
    url         = [ "http://yices.csl.sri.com/cgi-bin/yices2-newnewdownload.cgi\?file\=yices-2.4.1-src.tar.gz\&accept\=I+Agree", "yices-2.4.1-src.tar.gz" ]
    fmt         = "tar-gz-mv"
    build       = [ "./configure --prefix=`pwd`/../../../ext", "make -j%d" % multiprocessing.cpu_count() ]
    install     = [ "make install" ]

class package_efsmt:
    description = "Exists-Forall SMT Problem Solving"
    subdir      = "efsmt"
    url         = "http://www6.in.tum.de/~chengch/efsmt/efsmt_20130208.zip"
    fmt         = "zip"
    build       = [ "make lib" ]
    install     = [ "cp -v libefsmt.so ../../../ext/lib", "cp -v efsmt.h ../../../ext/include" ]

class package_aiger:
    description = "AIGER is a format, library and set of utilities for And-Inverter Graphs (AIGs)"
    subdir      = "aiger-1.9.9"
    url         = "http://fmv.jku.at/aiger/aiger-1.9.9.tar.gz"
    fmt         = "tar-gz"
    build       = [ "./configure.sh", "make -j%d" % multiprocessing.cpu_count() ]
    install     = [ "cp -v aigand aigdd aigflip aigfuzz aiginfo aigjoin aigmiter aigmove aignm aigor aigreset aigsim aigsplit aigstrip aigtoaig aigtoblif aigtocnf aigtodot aigtosmv aigunconstraint aigunroll aigvis andtoaig bliftoaig smvtoaig soltostim wrapstim %s" ]

class package_iimc:
    description = "iimc model checker for AIGs"
    subdir      = "iimc-2.0"
    url         = "ftp://vlsi.colorado.edu/pub/iimc/iimc-2.0.tar.gz"
    fmt         = "tar-gz"
    build       = [ "./configure", "make -j%d" % multiprocessing.cpu_count() ]
    install     = [ "cp -v iimc %s" ]

class package_cbmc_i386:
    description = "C Bounded Model Checker 5.0 (32bit)"
    subdir      = "cbmc-5.0-i386"
    url         = "http://www.cprover.org/cbmc/download/cbmc-5-0-linux-32.tgz"
    fmt         = "tgz"
    build       = [ "" ]
    install     = [ "cp -v cbmc goto-cc goto-instrument hw-cbmc %s" ]
    makedir     = True

class package_cbmc_x64:
    description = "C Bounded Model Checker 5.0 (64bit)"
    subdir      = "cbmc-5.0-x64"
    url         = "http://www.cprover.org/cbmc/download/cbmc-5-0-linux-64.tgz"
    fmt         = "tgz"
    build       = [ "" ]
    install     = [ "cp -v cbmc goto-cc goto-instrument hw-cbmc %s" ]
    makedir     = True

class package_demiurge:
    description = "SAT- and QBF-based AIG synthesizer for implementation from safety specifications"
    subdir      = "demiurge-1.1.0"
    url         = "http://www.iaik.tugraz.at/content/research/design_verification/demiurge/demiurge-1.1.0.tar.gz"
    fmt         = "tgz"
    build       = [ "" ]
    install     = [ "cp -v tool/build/src/demiurge-bin %s" ]
    makedir     = False

class package_xdot:
    description = "Interactive viewer for graphs written in Graphviz's dot language"
    subdir      = "xdot"
    url         = ["https://github.com/jrfonseca/xdot.py/raw/master/xdot.py"]
    fmt         = "wget-list"
    build       = [ "cp xdot.py xdot", "chmod +x xdot" ]
    install     = [ "cp -v xdot %s" ]

class package_cryptominisat:
    description = "An advanced SAT Solver"
    subdir      = "cryptominisat"
    url         = "https://github.com/msoos/cryptominisat"
    fmt         = "git"
    build       = [ "mkdir build", "cd build; cmake ..; make -j%d cryptominisat4" % multiprocessing.cpu_count() ]
    install     = [ "cp -v build/cryptominisat4 %s" ]

class package_bloqqer:
    description = "A preprocessor for quantified Boolean formulas"
    subdir      = "bloqqer-037-8660cb9-151127"
    url         = "http://fmv.jku.at/bloqqer/bloqqer-037-8660cb9-151127.tar.gz"
    fmt         = "tgz"
    build       = [ "./configure", "make" ]
    install     = [ "cp -v bloqqer %s" ]

# https://github.com/lonsing/depqbf/issues/7#issuecomment-159021610
class package_bloqqer_qdo:
    description = "Bloqqer configured to produce partial assignments (requires depqbf installed)"
    subdir      = "bloqqer-037-8660cb9-151127"
    url         = "http://fmv.jku.at/bloqqer/bloqqer-037-8660cb9-151127.tar.gz"
    fmt         = "tgz"
    build       = [ "sed -e \"s/DEPQBF=.*/DEPQBF=..\/depqbf-version-5.01\//\" configure > tmp", "cp tmp configure", "./configure -s", "make" ]
    install     = [ "mv bloqqer bloqqer-qdo", "cp -v bloqqer-qdo %s" ]

class package_depqbf:
    description = "A solver for quantified Boolean formulae (QBF)"
    subdir      = "depqbf-version-5.01"
    url         = "https://github.com/lonsing/depqbf/archive/version-5.01.tar.gz"
    fmt         = "tgz"
    build       = [ "make" ]
    install     = [ "cp -v depqbf %s" ]

class package_rareqs:
    description = "Recursive Abstraction Refinement QBF Solver"
    subdir      = "rareqs"
    url         = "http://sat.inesc-id.pt/~mikolas/sw/areqs/rareqs-1.1.tgz"
    fmt         = "tgz"
    build       = [ "" ]
    install     = [ "mv rareqs-1.1 rareqs", "cp -v rareqs %s" ]
    makedir     = True

class package_alejtehad:
    description = "A solver for quantified Boolean formulas based on counterexample-guided expansion"
    subdir      = "ijtihad"
    url         = "https://extgit.iaik.tugraz.at/scos/ijtihad.git"
    fmt         = "git"
    build       = [ "make" ]
    install     = [ "mv mysolver alejtehad", "cp -v alejtehad %s" ]

class package_cadet:
    description = "Algorithm for determining the truth of quantified Boolean formulas limited to a single quantifier alternations"
    subdir      = "cadet"
    url         = ["http://www.eecs.berkeley.edu/~rabe/cadet_linux"]
    fmt         = "wget-list"
    build       = [ "" ]
    install     = [ "mv cadet_linux cadet", "chmod a+x cadet", "cp -v cadet %s" ]

class package_skizzo:
    description = "A solver for deciding Quantified Boolean Formulas (QBFs)"
    subdir      = "skizzo"
    url         = ["http://skizzo.site/software/linux/sKizzo-v0.12"]
    fmt         = "wget-list"
    build       = [ "" ]
    install     = [ "mv sKizzo-v0.12 skizzo", "chmod a+x skizzo", "cp -v skizzo %s" ]
    makedir     = True

class package_gcc:
    description = "GCC, the GNU Compiler Collection"
    subdir      = "gcc-6.3.0"
    url         = "ftp://ftp.gwdg.de/pub/misc/gcc/releases/gcc-6.3.0/gcc-6.3.0.tar.gz"
    fmt         = "tgz"
    build       = ["./contrib/download_prerequisites", "mkdir build; cd build; ../configure --enable-languages=c,c++ --disable-multilib --prefix=../../../../ext; make -j%d" % multiprocessing.cpu_count()]
    install     = ["make install"]

class package_tpar:
    description = "A quantum circuit optimizer based on sum-over-paths representations"
    subdir      = "t-par"
    url         = "https://github.com/meamy/t-par.git"
    fmt         = "git"
    build       = [ "make -j%d" % multiprocessing.cpu_count() ]
    install     = [ "cp -v t-par %s" ]

################################################################################
# Foreign packages                                                             #
################################################################################

# Foreign packages are described as packages in here but are placed in separate
# files. They need to be located in the same directory as the tools.py script
# in filenames that end on _packages.py, e.g. my_packages.py,
# thirdparty_packages.py

for modulename in glob.glob( "%s/*_packages.py" % os.path.dirname( sys.argv[0] ) ):
    p = importlib.import_module( os.path.basename( modulename )[:-3] )
    for attr in [a for a in dir( p ) if a.startswith( "package_" )]:
        globals()[attr] = getattr( p, attr )

################################################################################
# Helper functions                                                             #
################################################################################

# from http://stackoverflow.com/questions/431684/how-do-i-cd-in-python
class cd:
    """Context manager for changing the current working directory"""
    def __init__(self, newPath):
        self.newPath = newPath

    def __enter__(self):
        self.savedPath = os.getcwd()
        os.chdir(self.newPath)

    def __exit__(self, etype, value, traceback):
        os.chdir(self.savedPath)

def checkout_or_download( package ):
    if package.fmt == "git":
        os.system( "git clone %s" % package.url )
    elif package.fmt == "hg":
        os.system( "hg clone %s" % package.url )
    elif package.fmt in ["tar-gz", "tgz"]:
        os.system( "wget %s" % package.url )
        if hasattr( package, "makedir" ) and package.makedir:
            subdir = package.subdir
            os.system( "mkdir {1}; mv `basename {0}` {1}".format(package.url,subdir) )
        else:
            subdir = "."
        os.system( "tar xfz {1}/`basename {0}` -C {1}".format(package.url,subdir) )
        os.system( "rm {1}/`basename {0}`".format(package.url,subdir) )
    elif package.fmt in ["zip"]:
        os.system( "wget %s" % package.url )
        os.system( "unzip `basename %s`" % package.url )
        os.system( "rm `basename %s`" % package.url )
    elif package.fmt in ["tar-gz-mv"]:
        os.system( "wget %s -O %s" % tuple( package.url ) )
        os.system( "tar xfz %s" % package.url[1] )
        os.system( "rm %s" % package.url[1] )
    elif package.fmt == "wget-list":
        os.mkdir( package.subdir )
        for u in package.url:
            os.system( "wget %s -O %s/`basename %s`" % ( u, package.subdir, u ) )
    else:
        print( "[e] unknown format: %s" % package.fmt )
        exit(1)

def get_name( package ):
    return next((k for k, v in globals().items() if v == package), None)[8:]

def patch( package, curpath ):
    patch_dir = "%s/patches/%s" % ( os.path.dirname( curpath + "/" + sys.argv[0] ), get_name( package ) )
    if os.path.exists( patch_dir ):
        for file in glob.glob( "%s/*.patch" % patch_dir ):
            os.system( "patch < %s" % file )

def update( package ):
    if package.fmt == "hg":
        os.system( "hg pull" )
        os.system( "hg update" )
    else:
        print( "[e] format does not support update: %s" % package.fmt )
        exit(1)

def build( package ):
    for cmd in package.build:
        os.system( cmd )

def install( package, curpath ):
    for cmd in package.install:
        if cmd.find( "%s" ) != -1:
            os.system( cmd % ( curpath + "/ext/bin" ) )
        else:
            os.system( cmd )

def build_and_install( package, curpath ):
    build( package )
    install( package, curpath )

################################################################################
# Command functions                                                            #
################################################################################
def cmd_install( package ):
    """Installs a package"""
    shutil.rmtree( "build/tools/%s" % package.subdir, ignore_errors = True )
    curpath = os.getcwd()
    with cd( "build/tools" ):
        checkout_or_download( package )
        with cd( package.subdir ):
            patch( package, curpath )
            build_and_install( package, curpath )

def cmd_update( package ):
    """Updates a package"""
    curpath = os.getcwd()
    with cd( "build/tools/%s" % package.subdir ):
        update( package )
        build_and_install( package, curpath )

def cmd_rebuild( package ):
    """Rebuilds a package"""
    curpath = os.getcwd()
    with cd( "build/tools/%s" % package.subdir ):
        build_and_install( package, curpath )

def cmd_commands():
    """Shows list of commands"""
    print( "Available commands:" )
    for k, v in globals().items():
        if k.startswith( "cmd_" ):
            name = "%s package" % k[4:] if len( inspect.getargspec( v ).args ) == 1 else k[4:]
            print( "  {:28s} {:s}".format( name, v.__doc__ ) )

def cmd_list():
    """Lists all available packages"""
    print( "Available packages:" )
    print( "\n".join( [ "  {:28s} {:s}".format( k[8:], v.description ) for k, v in globals().items() if k.startswith( "package_" ) ] ) )

################################################################################
# Main program                                                                 #
################################################################################
if __name__ == "__main__":
    # Setup some paths
    for path in [ "build/tools", "ext/bin" ]:
        if not os.path.exists( path ):
            os.makedirs( path )

    if len( sys.argv ) > 1:
        if ( not "cmd_%s" % sys.argv[1] in locals() ):
            print( "error: Unknown command '" + sys.argv[1] + "'" )
            sys.exit(0)

    if len( sys.argv ) == 2:
        locals()["cmd_%s" % sys.argv[1]]()
    elif len( sys.argv ) == 3:
        locals()["cmd_%s" % sys.argv[1]]( locals()["package_%s" % sys.argv[2]] )
    else:
        print( "usage: %s command [package_name]" % sys.argv[0] )
        print()
        print( "type '%s commands' to get a list of all commands" % sys.argv[0] )
