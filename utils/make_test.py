#!/usr/bin/env python
import os
import subprocess
import sys

from jinja2 import Environment

header = """/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

env = Environment()

template = env.from_string('''
{{ header }}

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE {{ basename }}

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(simple)
{



}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
''')

def make_test( header, basename ):
    return template.render( header = header, basename = basename ).strip()

if __name__ == "__main__":
    argc = len( sys.argv )
    if argc == 1 or argc > 3:
        print( "usage: make_test.py path/name [addon]" )
        exit( 1 )

    name   = sys.argv[1]
    addon  = sys.argv[2] if argc > 2 else "."

    basename = os.path.basename( name )

    pathname = "./" if addon == "." else "addons/cirkit-addon-%s/" % addon
    filename = pathname + "test/" + name

    with open( filename + ".cpp", "w" ) as f:
        f.write( make_test( header, basename ) + "\n" )

    os.system( "cmake build" )
