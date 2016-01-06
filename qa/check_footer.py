#!/usr/bin/env python3
import fnmatch
import os
from termcolor import colored

footer = """// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
"""

lines = footer.count( "\n" )

count = 0; total = 0
for _dir in [".", "addons/cirkit-addon-formal", "addons/cirkit-addon-reversible", "addons/cirkit-addon-experimental"]:
    for top in ["src", "programs", "test"]:
        for root, dirnames, filenames in os.walk( "{0}/{1}".format( _dir, top ) ):
            for filename in fnmatch.filter( filenames, "*.?pp" ):
                name = os.path.join( root, filename )
                file_footer = ''.join( open( name, "r" ).readlines()[-lines:] )
                if file_footer != footer:
                    count += 1
                    print( "{0} File {1} has wrong footer.".format( colored( '[E]', 'red', attrs = ['bold'] ), colored( name, 'green' ) ) )
                    print( "Found:" )
                    print( file_footer )
                    print( "Expected:" )
                    print( footer )
                total += 1

print( "{0} {1} from {2} files have a wrong footer.".format( colored( '[I]', 'white', attrs = ['bold'] ), count, total ) )
