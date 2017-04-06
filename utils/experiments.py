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
import re

def chunked( filename, commands_per_entry, offset = 0 ):
    data = json.load( open( filename, 'r' ) )
    if offset > 0:
        data = data[offset:]

    num_slices = int( ( len( data ) - 1 ) / commands_per_entry )
    return [data[i * commands_per_entry:(i * commands_per_entry) + commands_per_entry] for i in range( num_slices )]

class log_table_custom_entry:
    def __init__( self, value ):
        self.value = value

class log_table:
    def __init__( self, filename, commands_per_entry, offset = 0 ):
        self.headers = []
        self.columns = []
        self.defaults = {}
        self.column_size = 12

        self.slices = chunked( filename, commands_per_entry, offset )

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

    def to_csv( self, sep = ',' ):
        for slice in self.slices:
            print( sep.join( str( self.get_data( slice, column, index ) ) for index, column in enumerate( self.columns ) ) )

    def sort( self, func ):
        self.slices.sort( key = func )

    def set_default( self, column, value ):
        self.defaults[column] = value

    def get_data( self, slice, column, cid ):
        if isinstance( column, tuple ):
            index = column[0]
            key   = column[1]

            if key in slice[index]:
                data = slice[index][key]
            elif self.headers[cid] in self.defaults:
                data = self.defaults[self.headers[cid]]
            else:
                raise NameError( "Error for %s\nSlice: %s" % ( str( column ), slice ) )
            if len( column ) == 3:
                data = column[2]( data )
            return data
        elif isinstance( column, log_table_custom_entry ):
            return column.value

    def format_column( self, slice, column, cid ):
        return " %12s |" % self.get_data( slice, column, cid )

    # other utilities
    def sum( self, row, key ):
        return sum( slice[row][key] for slice in self.slices )

    def unique_count( self, row, key ):
        return len( set( slice[row][key] for slice in self.slices ) )

    # formatter
    @staticmethod
    def basename( value ):
        filename = value.split()[-1]
        return os.path.splitext( os.path.basename( filename ) )[0]

    def prec2( value ):
        return "%.2f" % float( value )

    @staticmethod
    def re_search( expr ):
        return lambda x : re.search( expr, x ).group( 1 )

class log_parser:
    def __init__( self, filename, commands_per_entry, offset = 0 ):
        self.slices = chunked( filename, commands_per_entry, offset )

    def __getitem__( self, key ):
        if isinstance( key, tuple ) and len( key ) == 2:
            slice_index, slice_key = key
            return [d[slice_index][slice_key] for d in self.slices]
        else:
            raise "I don't understand the key"

class fmt_combine:
    def __init__( self, *args ):
        self.fmts = args

    def __call__( self, value ):
        v = value
        for f in self.fmts:
            v = f( v )
        return v
