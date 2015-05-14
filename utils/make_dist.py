#!/usr/bin/env python

# CirKit: A circuit toolkit
# Copyright (C) 2009-2015  University of Bremen
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
# CREATES PACKAGES FROM CIRKIT AND ADDONS                                      #
################################################################################
#                                                                              #
# Author : Mathias Soeken (2015)                                               #
#                                                                              #
# This is a script that generates packages from cirkit that are configured     #
# through a simple YAML file.  Using this script, e.g., the RevKit packages    #
# are created.                                                                 #
#                                                                              #
################################################################################

import fnmatch
import itertools
import os
import re
import shutil
import subprocess
import sys
import yaml
from termcolor import colored

################################################################################
# Helper functions                                                             #
################################################################################

def command( cmd ):
    return subprocess.check_output( cmd, shell = True ).decode().strip()

def error( str, will_quit = False ):
    print( "%s %s" % ( colored( '[e]', 'red', attrs = ['bold'] ), str ) )
    if will_quit: sys.exit( 1 )

def info( str ):
    print( "%s %s" % ( colored( '[i]', 'white', attrs = ['bold'] ), str ) )

def files( path, wildcard ):
    _f = ( ( root, filename ) for root, dirnames, filenames in os.walk( path ) for filename in fnmatch.filter( filenames, wildcard ) )
    return itertools.groupby( _f, lambda a: a[0] )

def add_file( cfg, base, file ):
    name = os.path.join( base, file )
    if "only" in cfg:
        for p in cfg['only']:
            if re.match( p, name ): return True
        return False
    return True

################################################################################
# Functions                                                                    #
################################################################################

base_files = [ "CMakeLists.txt", "README.md", "ext/CMakeLists.txt", "addons/CMakeLists.txt", "src/CMakeLists.txt", "programs/CMakeLists.txt", "test/CMakeLists.txt", ( "src", "*.?pp" ), ( "programs", "*.?pp" ), ( "test", "*.?pp" ), ( "cmake", "*.cmake" ), ( "cmake", "nothing.cpp" )  ]
addon_files = [ ( "src", "*.?pp" ), ( "programs", "*.?pp" ), ( "test", "*.?pp" )  ]

def make_dist( config ):
    if "name" not in config: error( "name has not been specified", True )
    name = config['name']
    version = config['version'] if "version" in config else command( "cat src/core/version.cpp | grep static | awk '{print $6}' | sed -e \"s/[\\\";]//g\"" )
    info( "make package for %s %s" % ( colored( name, 'green' ), colored( version, 'green' ) ) )

    # Create directory
    path = "%s-%s" % ( name, version )
    archive = "%s.tar.gz" % path
    if os.path.exists( path ): shutil.rmtree( path )
    if os.path.exists( archive ): os.remove( archive )
    os.mkdir( path )

    # Copy base files
    info( "copy base files" )
    for pattern in base_files:
        if type( pattern ) is tuple:
            for p, g in files( *pattern ):
                destpath = os.path.join( path, p )
                if not os.path.exists( destpath ): os.makedirs( destpath )
                for _, f in g:
                    shutil.copy( os.path.join( p, f ), os.path.join( destpath, f ) )
        else:
            p, file = os.path.split( pattern )
            destpath = os.path.join( path, p )
            if not os.path.exists( destpath ): os.makedirs( destpath )
            shutil.copy( pattern, os.path.join( destpath, file ) )

    # Merge addon
    if "merge_addon" in config:
        ma = config['merge_addon']
        if "name" not in ma: error( "name has not been specified in merge_addon", True )
        aname = ma['name']
        info( "merge addon %s" % ( colored( aname, 'green' ) ) )

        # Copy addon files
        apath = "addons/cirkit-addon-%s" % aname
        for base, pattern in addon_files:
            for p, g in files( os.path.join( apath, base ), pattern ):
                destpath = os.path.join( path, p[len(apath) + 1:] )
                copy_files = [f for _, f in g if add_file( ma, p[len(apath) + 1:], f )]
                if len( copy_files ) != 0:
                    if not os.path.exists( destpath ): os.makedirs( destpath )
                    for f in copy_files:
                        shutil.copy( os.path.join( p, f ), os.path.join( destpath, f ) )

        # Merge CMakeLists.txt
        if "merge_cmake" in ma:
            mc = ma['merge_cmake']
            dest = os.path.join( mc['dest'] if "dest" in mc else "ext", "CMakeLists.txt" )

            cmake = open( os.path.join( apath, "CMakeLists.txt" ), "r" ).readlines()

            if "overrides" in mc:
                for ov in mc['overrides']:
                    cmake[int(ov['line']) - 1] = ov['new'] + "\n"

            if "lines" in mc:
                lines = [int( l ) - 1 for l in mc['lines'].split( "-" )]
                cmake = cmake[lines[0]:lines[1]]

            with open( os.path.join( path, dest ), "a" ) as f:
                f.write( "\n" )
                for c in cmake:
                    f.write( c )

    # Merge README?
    if "readme_extra" in config:
        info( "merge README.md" )
        with open( os.path.join( path, "README.md" ), "a" ) as f:
            f.write( "\n" )
            f.write( config['readme_extra'] )

    # Merge extras?
    if "merge_extra" in config:
        for extra in config['merge_extra']:
            info( "merge %s" % colored( extra, 'green' ) )
            command( "tar xfz %s -C %s" % ( extra, path ) )

    # Package manager?
    if "package_manager" in config and config['package_manager']:
        info( "copy package manager" )
        utils = os.path.join( path, "utils" )
        if not os.path.exists( utils ): os.makedirs( utils )
        shutil.copy( "utils/tools.py", os.path.join( utils, "tools.py" ) )
        shutil.copytree( "utils/patches", os.path.join( utils, "patches" ) )

    # Rename stuff?
    if "header_title" in config:
        info( "change header title" )
        command( "find %s -type f | xargs sed -i -e \"s/CirKit: A circuit toolkit/%s/g\"" % ( path, config['header_title'] ) )

    if "title" in config:
        info( "rename title" )
        command( "find %s -type f | xargs sed -i -e \"s/CirKit/%s/g\"" % ( path, config['title'] ) )

        command( "sed -i -e \"s/%sTools/CirKitTools/g\" %s-1.1/CMakeLists.txt" % ( config['title'], config['name'] ) )

    if "namespace" in config:
        info( "rename namespace" )
        command( "find %s -type f | xargs sed -i -e \"s/cirkit/%s/g\"" % ( path, config['namespace'] ) )

    if "version" in config:
        info( "override version" )
        command( "find %s -type f | xargs sed -i -e \"s/@since.*$/@since %s/g\"" % ( path, config['version'] ) )

    # Create archive
    info( "create archive %s" % colored( archive, 'green' ) )
    command( "tar cfz {0}.tar.gz {0}".format( path ) )

################################################################################
# Main entry point                                                             #
################################################################################

if __name__ == "__main__":
    argc = len( sys.argv )
    if argc != 2:
        print( "usage: make_dist.py config.yaml" )
        exit( 1 )

    config = sys.argv[1]

    with open( config, "r" ) as f:
        y = yaml.load_all( f )
        for doc in y:
            make_dist( doc )
