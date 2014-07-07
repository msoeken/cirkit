/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2014  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include <iostream>

#include <reversible/truth_table.hpp>
#include <reversible/io/write_pla.hpp>
#include <reversible/utils/program_options.hpp>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/adaptors.hpp>

using namespace cirkit;

void parse_string_list( const std::string& s, std::vector<unsigned>& list )
{
  using boost::adaptors::transformed;

  std::vector<std::string> slist;
  boost::split( slist, s, boost::is_any_of( " " ) );
  boost::push_back( list, slist | transformed( []( const std::string& s ) { return boost::lexical_cast<unsigned>( s ); } ) );
}

void permutation_to_truth_table( const std::vector<unsigned>& list, binary_truth_table& spec )
{
  unsigned bw = (unsigned)( log( list.size() ) / log( 2.0 ) );

  unsigned in = 0u;
  for ( const auto& out : list )
  {
    spec.add_entry( number_to_truth_table_cube( in, bw ), number_to_truth_table_cube( out, bw ) );
    ++in;
  }
}

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  std::string permutation;
  std::string filename;

  program_options opts;
  opts.add_options()
    ( "permutation",         value<std::string>( &permutation ), "Permutation (starts with 0, space separated)" )
    ( "print_truth_table,t",                                     "Prints the truth table of the circuit"        )
    ( "filename",            value<std::string>( &filename    ), "PLA filename"                                 )
    ;

  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_set( "permutation" ) )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  std::vector<unsigned> perm_list;
  binary_truth_table spec;

  parse_string_list( permutation, perm_list );
  permutation_to_truth_table( perm_list, spec );

  if ( opts.is_set( "print_truth_table" ) )
  {
    std::cout << spec << std::endl;
  }

  if ( opts.is_set( "filename" ) )
  {
    write_pla( spec, filename );
  }

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// End:
