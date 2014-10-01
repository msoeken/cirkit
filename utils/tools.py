#!/usr/bin/env python

# CirKit: A circuit toolkit
# Copyright (C) 2009-2014  University of Bremen
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
# that one as simple as possible.                                              #
#                                                                              #
# New packages can be added in the Packages section.  The flow works for the   #
# most common scenarios. A new scenario usually only requires the addition of  #
# a new fmt entry together with its handling in the checkout_or_download       #
# function                                                                     #
#                                                                              #
################################################################################

import inspect
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
#               tar-gz    : Downloads url and uses tar xvfz for unpacking
#               wget-list : url must be a list, all files are downloaded and
#                           placed into subdir
# build       - A list of commands that should be executed for building
# install     - A list of commands that should be executed for installing
#               One can place a "%s" into a command to refer to the ext/bin
#               directory

class package_abc:
    description = "ABC: System for Sequential Logic Synthesis and Formal Verification"
    subdir      = "abc"
    url         = "https://bitbucket.org/alanmi/abc"
    fmt         = "hg"
    build       = [ "make -j5" ]
    install     = [ "cp -v abc %s" ]

class package_minisat:
    description = "Minimalistic, open-source SAT solver"
    subdir      = "minisat"
    url         = "http://minisat.se/downloads/minisat-2.2.0.tar.gz"
    fmt         = "tar-gz"
    build       = [ "MROOT=`pwd` make -C core rs" ]
    install     = [ "cp -v core/minisat_static %s/minisat" ]

class package_sat13:
    description = "Knuth's CDCL SAT solver"
    subdir      = "sat13"
    url         = [ "http://www-cs-faculty.stanford.edu/~uno/programs/sat13.w", "http://tug.ctan.org/support/graphbase/gb_flip.w", "http://tug.ctan.org/support/graphbase/boilerplate.w"]
    fmt         = "wget-list"
    build       = [ "ctangle gb_flip.w", "ctangle sat13.w", "gcc -o sat13 gb_flip.c sat13.c" ]
    install     = [ "cp -v sat13 %s" ]

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
    if package.fmt == "hg":
        os.system( "hg clone %s" % package.url )
    elif package.fmt == "tar-gz":
        os.system( "wget %s" % package.url )
        os.system( "tar xvfz `basename %s`" % package.url )
        os.system( "rm `basename %s`" % package.url )
    elif package.fmt == "wget-list":
        os.mkdir( package.subdir )
        for u in package.url:
            os.system( "wget %s -O %s/`basename %s`" % ( u, package.subdir, u ) )
    else:
        print( "[e] unknown format: %s" % package.fmt )

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
            for cmd in package.build:
                os.system( cmd )
            for cmd in package.install:
                if cmd.find( "%s" ) != -1:
                    os.system( cmd % ( curpath + "/ext/bin" ) )
                else:
                    os.system( cmd )

def cmd_update( package ):
    """Updates a package (not yet implemented)"""
    pass

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
            os.mkdir( path )

    if len( sys.argv ) == 2:
        locals()["cmd_%s" % sys.argv[1]]()
    elif len( sys.argv ) == 3:
        locals()["cmd_%s" % sys.argv[1]]( locals()["package_%s" % sys.argv[2]] )
    else:
        print( "usage: %s command [package_name]" % sys.argv[0] )
