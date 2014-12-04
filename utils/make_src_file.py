#!/usr/bin/env python
import os
import subprocess
import sys

from jinja2 import Environment

header_revkit = """/* RevKit (www.revkit.org)
 * Copyright (C) 2009-2014  University of Bremen
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

header_cirkit = """/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2014  University of Bremen
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
header_template = env.from_string('''
{{ header }}
/**
 * @file {{ basename }}.hpp
 *
 * @brief TODO
 *
 * @author {{ author }}
 * @since  2.2
 */

#ifndef {{ basename.upper() }}_HPP
#define {{ basename.upper() }}_HPP

namespace cirkit
{



}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
''')

source_template = env.from_string('''
{{ header }}
#include "{{ basename }}.hpp"

namespace cirkit
{



}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
''')

def git_user_name():
    return subprocess.check_output( ["git", "config", "--global", "user.name"] ).decode( "utf-8" ).strip()

def make_header( header, basename, author ):
    return header_template.render( header = header, basename = basename, author = author ).strip()

def make_source( header, basename ):
    return source_template.render( header = header, basename = basename ).strip()

if __name__ == "__main__":
    argc = len( sys.argv )
    if argc == 1 or argc > 4:
        print( "usage: make_src_file.py path/name [addon [author]]" )

    name   = sys.argv[1]
    addon  = sys.argv[2] if argc > 2 else "."
    author = sys.argv[3] if argc > 3 else git_user_name()

    header   = header_revkit if addon == "." else header_cirkit
    basename = os.path.basename( name )

    pathname = "./" if addon == "." else "addons/cirkit-addon-%s/" % addon
    filename = pathname + "src/" + name

    with open( filename + ".hpp", "w" ) as f:
        f.write( make_header( header, basename, author ) + "\n" )

    with open( filename + ".cpp", "w" ) as f:
        f.write( make_source( header, basename ) + "\n" )
