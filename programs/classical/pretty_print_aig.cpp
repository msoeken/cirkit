/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2017  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
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
