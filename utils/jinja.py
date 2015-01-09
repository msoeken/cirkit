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
# JINJA STANDALONE WRAPPER                                                     #
################################################################################
#                                                                              #
# Author : Mathias Soeken (2015)                                               #
#                                                                              #
# This file helps to use jinja2 templates for standalone use, e.g. in          #
# benchmarking scripts.  The first parameter to the file is a jinja file that  #
# contains the plaintext using the jinja2 template syntax.  The second         #
# parameter is JSON data to represent the parameter that are passed and used   #
# in the template.  The third parameter is optional; if given the output is    #
# written to that location, otherwise the output is dumped to stdout.          #
#                                                                              #
################################################################################

import json
import sys

from jinja2 import Environment

if __name__ == "__main__":
    argc = len( sys.argv )
    if argc < 3 or argc > 4:
        print( "usage: jinja.py template json [filename]" )
        exit( 1 )

    template = sys.argv[1]
    jsondata = sys.argv[2]
    filename = sys.argv[3] if argc > 3 else "-"

    env = Environment()
    with open( template, "r" ) as f:
        t = env.from_string( f.read() )

    output = t.render( json.loads( jsondata ) )

    if filename == "-":
        print( output )
    else:
        with open( filename, "w" ) as f:
            f.write( output )
            f.write( "\n" )

