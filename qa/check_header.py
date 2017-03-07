#!/usr/bin/env python3
import fnmatch
import os
from termcolor import colored

header_cirkit = """/* {tool}
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2017  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
"""

def get_source_files( *addon_names, directories = [ "src", "programs", "test" ], filt = "*.?pp" ):
    # base dirs
    base_dirs = ["."] + ["addons/cirkit-addon-%s" % a for a in addon_names]
    # refine
    base_dirs = [ os.path.join( base, top ) for base in base_dirs for top in directories ]
    # now walk through them
    return [os.path.join(root, filename) for base_dir in base_dirs for root, _, filenames in os.walk( base_dir ) for filename in fnmatch.filter( filenames, filt )]

def matches( file_header, header ):
    for tool in ["CirKit: A circuit toolkit", "alice: A C++ EDA command line interface API"]:
        if header.replace( "{tool}", tool ) == file_header:
            return True
    return False

count = 0
total = 0
header = header_cirkit
lines = header.count( "\n" )

for name in get_source_files( "formal", "reversible", "experimental" ):
    ll = open( name, "r" ).readlines()
    if ll[0].strip() == "/* [custom_license] */":
        total += 1
        continue

    while len( ll ) > lines and ll[lines - 1].strip() != '*/':
        ll.remove( ll[3] )

    file_header = ''.join( ll[0:lines] )
    if not matches( file_header, header ):
        count += 1
        print( "{0} File {1} has wrong header.".format( colored( '[e]', 'red', attrs = ['bold'] ), colored( name, 'green' ) ) )
    total += 1

print( "{0} {1} from {2} files have a wrong header.".format( colored( '[i]', 'white', attrs = ['bold'] ), count, total ) )
