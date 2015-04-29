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

  std::cout << format( "Run-time:   %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;
  std::cout << "Node count:     " << Cudd_ReadNodeCount( bdd.cudd ) << std::endl;
  std::cout << "Level sizes:    " << any_join( level_sizes( bdd.cudd, voutputs ), " " ) << std::endl;
  std::cout << "Maximum fanout: " << maximum_fanout( bdd.cudd, voutputs ) << std::endl;
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
