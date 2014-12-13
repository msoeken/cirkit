#!/usr/bin/env python
import fnmatch
import os
from termcolor import colored

count = 0; total = 0
for top in [".", "addons/cirkit-addon-formal", "addons/cirkit-addon-mini", "addons/cirkit-addon-revlib", "addons/cirkit-addon-experimental", "addons/cirkit-addon-verific", "addons/cirkit-addon-yosys"]:
    for _dir, suffix in [("src", "*.hpp"), ("programs", "*.cpp")]:
        for root, dirnames, filenames in os.walk( top + "/" + _dir ):
            for filename in fnmatch.filter( filenames, suffix ):
                name = os.path.join( root, filename )
                lines = [line for line in open( name, "r" ).readlines() if line.find( "@author" ) >= 0]

                if len( lines ) == 0:
                    print( "{0} File {1} does not contain an author.".format( colored( '[E]', 'red', attrs = ['bold'] ), colored( name, 'green' ) ) )
                    count += 1
                total += 1

print( "{0} {1} from {2} files do not contain an author.".format( colored( '[I]', 'white', attrs = ['bold'] ), count, total ) )
