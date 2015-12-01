################################################################################
# experiments.py                                                               #
################################################################################
#                                                                              #
# Author : Mathias Soeken (2015)                                               #
#                                                                              #
# These are some useful Python scripts to help with setting up experiments.    #
# They make use of the logging feature in CirKit.                              #
#                                                                              #
# Suggested usage:                                                             #
# Define an environment variable CIRKIT_HOME that points to the base directory #
# of cirkit and then add the following to the Python file for experiments:     #
#                                                                              #
# import os                                                                    #
# import sys                                                                   #
# sys.path.append( os.getenv( 'CIRKIT_HOME' ) + "/utils/" )                    #
# from experiments import log_table                                            #
################################################################################

import json
import os

class log_table:
    def __init__( self, filename, commands_per_entry, offset = 0 ):
        data = json.load( open( filename, 'r' ) )
        self.headers = []
        self.columns = []
        self.defaults = {}

        if offset > 0:
            data = data[offset:]

        num_slices = int( ( len( data ) - 1 ) / commands_per_entry )
        self.slices = [data[i * commands_per_entry:(i * commands_per_entry) + commands_per_entry] for i in range( num_slices )]

    def __setitem__( self, key, value ):
        self.headers.append( key )
        self.columns.append( value )

    def __repr__( self ):
        str = "|"
        for header in self.headers:
            str += " %12s |" % header
        str += "\n"

        for slice in self.slices:
            str += "|"
            for index, column in enumerate( self.columns ):
                str += self.format_column( slice, column, index )
            str += "\n"

        return str

    def set_default( self, column, value ):
        self.defaults[column] = value

    def format_column( self, slice, column, cid ):
        if isinstance( column, tuple ):
            index = column[0]
            key   = column[1]

            if key in slice[index]:
                data = slice[index][key]
            elif self.headers[cid] in self.defaults:
                data = self.defaults[self.headers[cid]]
            else:
                raise "Error"
            if len( column ) == 3:
                data = column[2]( data )
            return " %12s |" % data

    # formatter
    @staticmethod
    def basename( value ):
        filename = value.split()[-1]
        return os.path.splitext( os.path.basename( filename ) )[0]

