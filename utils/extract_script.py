#!/usr/bin/env python3
import json
import sys

if len( sys.argv ) != 2:
    print( "usage: %s logfile" % sys.argv[0] )
    sys.exit( 1 )

for command in json.load( open( sys.argv[1] ) ):
    print( command['command'] )
