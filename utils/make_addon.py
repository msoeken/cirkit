#!/usr/bin/env python3
import os
import sys

from jinja2 import Environment

env = Environment()
cmake_template = env.from_string("""add_cirkit_library(
  NAME cirkit_{{ lcase }}
  AUTO_DIRS src
  USE
    cirkit_classical
  INCLUDE
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
  DEFINE
    PUBLIC ADDON_{{ ucase }}
)
""")

def make_header( header, basename, author ):
    return header_template.render( header = header, basename = basename, author = author ).strip()

def make_source( header, basename ):
    return source_template.render( header = header, basename = basename ).strip()

if __name__ == "__main__":
    argc = len( sys.argv )
    if argc != 2:
        print( "usage: ./utils/make_addon.py addon" )
        exit( 1 )

    name   = sys.argv[1]
    lcase  = name.lower()
    ucase  = name.upper()

    path   = "addons/cirkit-addon-" + lcase
    if os.path.exists( path ):
        print( "addon %s already exists" % lcase )
        exit( 1 )

    os.makedirs( path )
    os.makedirs( path + "/src" )

    with open( path + "/CMakeLists.txt", "w" ) as f:
        f.write( cmake_template.render( lcase = lcase, ucase = ucase ).strip() + "\n" )

    print( "[i] added addon" )
    print( "[i] reconfigure CirKit" )
    os.system( "cmake build -Denable_cirkit-addon-%s=ON" % lcase )

    print( "[i] you can now add files to the addon. For example:" )
    print( "    mkdir addons/cirkit-addon-%s/src/classical/" % lcase )
    print( "    ./utils/make_src_file.py classical/algorithm %s" % lcase )

