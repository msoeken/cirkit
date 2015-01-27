/* CirKit: A circuit toolkit
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

/**
 * @author Heinz Riener
 */

#include <core/utils/program_options.hpp>
#include <classical/aig.hpp>
#include <classical/io/read_aiger.hpp>
#include <classical/io/write_aiger.hpp>
#include <classical/utils/aig_utils.hpp>

#if ADDON_GRAPHVIZ
#include <classical/io/write_graphviz.hpp>
#include <classical/io/write_tex.hpp>
#endif

using namespace cirkit;

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  std::string filename;
#if ADDON_GRAPHVIZ
  std::string layout_algorithm;
  std::string render_format;
#endif

  program_options opts;
  opts.add_options()
    ( "filename",          value( &filename ),         "AIG filename (in ASCII AIGER format)" )
    ( "dot,d",                                         "Write dot (using boost)" )
#if ADDON_GRAPHVIZ
    ( "graphviz,g",                                    "Write GraphViz" )
    ( "tex,t",                                         "Write TeX (using GraphViz)" )
    ( "layout",            value( &layout_algorithm ), "Layout algorithm (for GraphViz and TeX output, default:dot)" )
    ( "format",            value( &render_format ),    "Render format (for GraphViz, default:dot)" )
#endif
    ( "fill-sym-table,f",                              "Fill missing symbol table entries with generic names" )
    ( "mirror,m",                                      "Mirror word-level names" )
    ;
  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_set( "filename" ) )
  {
    std::cout << opts << std::endl;
    return 1;
  }

#if ADDON_GRAPHVIZ
  /* set default arguments */
  if ( ( opts.is_set( "graphviz" ) || opts.is_set( "tex" ) ) && !opts.is_set( "layout" ) )
  {
    layout_algorithm = "dot";
  }

  if ( opts.is_set( "graphviz" ) && !opts.is_set( "format" ) )
  {
    render_format = "dot";
  }
#endif

  std::string comment;
  aig_graph aig;
  try
  {
    read_aiger( aig, comment, filename );
  }
  catch ( const char* msg )
  {
    std::cerr << msg << std::endl;
    return 2;
  }

  if ( opts.is_set( "mirror" ) )
  {
    aig_mirror_word_names( aig );
  }

  try
  {
    if ( opts.is_set( "dot" ) )
    {
      write_dot( aig, std::cout );
    }
#if ADDON_GRAPHVIZ
    else if ( opts.is_set( "graphviz" ) )
    {
      write_graphviz( aig, layout_algorithm, render_format, std::cout );
    }
    else if ( opts.is_set( "tex" )  )
    {
      write_tex( aig, layout_algorithm, std::cout, opts.is_set( "fill-sym-table" ) );
    }
#endif
    else
    {
      write_aiger( aig, std::cout, opts.is_set( "fill-sym-table" ) );
      if ( comment != "" ) {
        std::cout << comment << '\n';
      }
    }
  }
  catch ( const char* msg )
  {
    std::cerr << msg << std::endl;
    return 2;
  }

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
