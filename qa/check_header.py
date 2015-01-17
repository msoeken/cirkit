#!/usr/bin/env python
import fnmatch
import os
from termcolor import colored

header_revkit = """/* RevKit (www.revkit.org)
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

header_cirkit = """/* CirKit: A circuit toolkit
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

count = 0; total = 0
for _dirs, header in [([".", "addons/cirkit-addon-formal", "addons/cirkit-addon-mini", "addons/cirkit-addon-revlib"], header_revkit), \
                      (["addons/cirkit-addon-experimental", "addons/cirkit-addon-verific", "addons/cirkit-addon-yosys"], header_cirkit)]:
    lines = header.count( "\n" )
    for _dir in _dirs:
        for top in ["src", "programs", "test"]:
            for root, dirnames, filenames in os.walk( "{0}/{1}".format( _dir, top ) ):
                for filename in fnmatch.filter( filenames, "*.?pp" ):
                    name = os.path.join( root, filename )
                    file_header = ''.join( open( name, "r" ).readlines()[0:lines] )
                    if file_header != header:
                        count += 1
                        print( "{0} File {1} has wrong header.".format( colored( '[E]', 'red', attrs = ['bold'] ), colored( name, 'green' ) ) )
                    total += 1

print( "{0} {1} from {2} files have a wrong header.".format( colored( '[I]', 'white', attrs = ['bold'] ), count, total ) )
