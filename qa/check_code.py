#!/usr/bin/env python
import fnmatch
import os
import re
import sys
from termcolor import colored
from textwrap import dedent
from jinja2 import Environment

forbidden_boost_headers = [ "foreach", "bind", "lambda/lambda", "shared_ptr", "regex", "tuple/tuple", "range/irange" ]

def check_for_string( id, line, str, explanation = None ):
    if re.search( str, line ):
        suffix = colored( ' [ignored]', 'yellow', attrs = ['bold'] ) if line.endswith( "//:QA_IGN\n" ) else ""
        info = explanation if explanation else str
        print( "{0} {1} File {2} contains {3}{4}".format( colored( '[E]', 'red', attrs = ['bold'] ),
                                                          colored( "W%s" % id, 'red' ),
                                                          colored( name, 'green' ),
                                                          colored( info, 'blue' ), suffix ) )
        return True
    return False

def check_file( name ):
    found = False
    with open( name, "r" ) as f:
        for line in f:
            for header in forbidden_boost_headers:
                found = found or check_for_string( 1, line, "boost/%s.hpp" % header )
            found = found or check_for_string( 2, line, "shared_ptr\(.*new" )
            found = found or check_for_string( 2, line, "::ptr\(.*new" )
            found = found or check_for_string( 3, line, "typedef" )
            found = found or check_for_string( 4, line, "enum +([^c]|c[^l]|cl[^a]|cla[^s]|clas[^s])", "unscoped enum" )
    return not found

def print_help( id = None ):
    infos = [
        ( "Obsolete Boost headers", '''
          For some Boost libraries alternatives exist in C++11 and hence they
          should not be used anymore.  Use the following replacements.

          Boost.Bind            C++ lambda expressions
          Boost.Foreach         Range-based for loop
          Boost.irange          Boost.counting_range
          Boost.Lambda          C++ lambda expressions
          Boost Smart Pointers  C++ smart pointers (std::shared_ptr, std::unique_ptr, ...)
          Boost.Regex           std::regex
          Boost.Tuple           std::tuple
        '''),
        ( "Initialization of smart pointers", '''
          Do not initialize smart pointers with their standard constructor and `new T`
          but use `std::make_shared<T>` or `std::make_unique<T>` instead.
        '''),
        ( "Do not use typedef's", '''
          Instead of `typedef T A` use type aliases with `using A = T`.
        '''),
        ( "Do not use unscoped enum's", '''
           Instead of `enum E` use `enum class E` and an explicit scope.
        ''' )
    ]

    for index, (title, body) in enumerate( infos ):
        if id and ( index + 1 ) != id: continue
        print( colored( "W%s: %s" % ((index + 1), title), 'white', attrs = ['bold'] ) )
        print()
        print( dedent( body ).strip() )
        print()

if len( sys.argv ) in [2, 3] and sys.argv[1] == '-h':
    print_help( int( sys.argv[2] ) if len( sys.argv ) == 3 else None )
else:
    count = 0; total = 0
    for _dir in [".", "addons/cirkit-addon-formal", "addons/cirkit-addon-mini", "addons/cirkit-addon-revlib", "addons/cirkit-addon-reversible", "addons/cirkit-addon-experimental", "addons/cirkit-addon-verific", "addons/cirkit-addon-yosys"]:
        for top in ["src", "programs", "test"]:
            for root, dirnames, filenames in os.walk( "{0}/{1}".format( _dir, top ) ):
                for filename in fnmatch.filter( filenames, "*.?pp" ):
                    name = os.path.join( root, filename )
                    if not check_file( name ):
                        count += 1
                    total += 1

    print( "{0} {1} from {2} files have some code issues. Use -h [id] for an explanation of the warning ids.".format( colored( '[I]', 'white', attrs = ['bold'] ), count, total ) )
