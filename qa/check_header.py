#!/usr/bin/env python3
import fnmatch
import os
from termcolor import colored

header_cirkit = """/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
"""

def get_source_files( *addon_names, directories = [ "src", "programs", "test" ], filt = "*.?pp" ):
    # base dirs
    base_dirs = ["."] + ["addons/cirkit-addon-%s" % a for a in addon_names]
    # refine
    base_dirs = [ os.path.join( base, top ) for base in base_dirs for top in directories ]
    # now walk through them
    return [os.path.join(root, filename) for base_dir in base_dirs for root, _, filenames in os.walk( base_dir ) for filename in fnmatch.filter( filenames, filt )]

count = 0
total = 0
header = header_cirkit
lines = header.count( "\n" )

for name in get_source_files( "formal", "reversible", "experimental" ):
    file_header = ''.join( open( name, "r" ).readlines()[0:lines] )
    if file_header != header:
        count += 1
        print( "{0} File {1} has wrong header.".format( colored( '[e]', 'red', attrs = ['bold'] ), colored( name, 'green' ) ) )
    total += 1

print( "{0} {1} from {2} files have a wrong header.".format( colored( '[i]', 'white', attrs = ['bold'] ), count, total ) )
