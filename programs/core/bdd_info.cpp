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
 * @author Mathias Soeken
 */

#include <cstdio>
#include <functional>
#include <iostream>
#include <vector>

#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>

#include <core/io/read_pla_to_bdd.hpp>
#include <core/utils/bdd_utils.hpp>
#include <core/utils/program_options.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/string_utils.hpp>

#include <cuddObj.hh>

using namespace cirkit;

int main( int argc, char ** argv )
{
  using boost::format;
  using boost::program_options::value;

  std::string filename;
  std::string ordering;
  std::string dotname;

  program_options opts;
  opts.add_options()
    ( "filename",  value( &filename ), "PLA filename" )
    ( "ordering",  value( &ordering ), "Complete variable ordering (space separated)" )
    ( "dotname",   value( &dotname ),  "Writes BDD to this file" )
    ( "dumpadd,a",                     "Dumps BDD without complement edges" )
    ( "verbose,v",                     "Be verbose" )
    ;

  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_set( "filename" ) )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  std::vector<unsigned> vordering;
  parse_string_list( vordering, ordering );
  auto settings = std::make_shared<properties>();
  settings->set( "ordering", vordering );
  auto statistics = std::make_shared<properties>();

  BDDTable bdd;
  read_pla_to_bdd( bdd, filename, settings, statistics );

  auto voutputs = get_map_values( bdd.outputs );

  std::cout << format( "Run-time:         %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;
  std::cout << "Node count:       " << Cudd_ReadNodeCount( bdd.cudd ) << std::endl;
  std::cout << "Level sizes:      " << any_join( level_sizes( bdd.cudd, voutputs ), " " ) << std::endl;
  std::cout << "Maximum fanout:   " << maximum_fanout( bdd.cudd, voutputs ) << std::endl;
  std::cout << "Complement edges: " << count_complement_edges( bdd.cudd, voutputs ) << std::endl;
  for ( const auto& p : bdd.outputs )
  {
    std::cout << "Info for output " << p.first << ":" << std::endl;
    std::cout << "- Path count:               " << Cudd_CountPath( p.second ) << std::endl;
    std::cout << "- Path count (to non-zero): " << Cudd_CountPathsToNonZero( p.second ) << std::endl;
  }

  if ( !dotname.empty() )
  {
    using namespace std::placeholders;

    FILE * fd = fopen( dotname.c_str(), "w" );

    auto rinames = get_map_keys( bdd.inputs );
    auto ronames = get_map_keys( bdd.outputs );
    auto outputs = get_map_values( bdd.outputs );

    if ( opts.is_set( "dumpadd" ) )
    {
      boost::transform( outputs, outputs.begin(),  std::bind( Cudd_BddToAdd, bdd.cudd, _1 ) );
    }

    std::vector<char*> inames( bdd.inputs.size() ), onames( outputs.size() );
    boost::transform( rinames, inames.begin(), []( const std::string& s ) { return const_cast<char*>( s.c_str() ); } );
    boost::transform( ronames, onames.begin(), []( const std::string& s ) { return const_cast<char*>( s.c_str() ); } );

    Cudd_DumpDot( bdd.cudd, outputs.size(), &outputs[0], &inames[0], &onames[0], fd );

    fclose( fd );
  }

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
