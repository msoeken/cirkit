/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

#include "stores.hpp"

#include <cstdio>
#include <functional>
#include <iostream>

#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>

#include <core/utils/range_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

template<>
std::string store_entry_to_string<bdd_function_t>( const bdd_function_t& bdd )
{
  return ( boost::format( "%d variables, %d functions, %d nodes" ) % bdd.first.ReadSize() % bdd.second.size() % bdd.first.ReadKeys() ).str();
}

template<>
void print_store_entry<bdd_function_t>( std::ostream& os, const bdd_function_t& bdd )
{
  for ( const auto& f : index( bdd.second ) )
  {
    os << "Function " << f.index << std::endl;
    f.value.PrintMinterm();
    os << std::endl;
  }
}

show_store_entry<bdd_function_t>::show_store_entry( cli_options& opts )
{
  boost::program_options::options_description bdd_options( "BDD options" );

  bdd_options.add_options()
    ( "add", "Convert BDD to ADD to have no complemented edges" )
    ;

  opts.opts.add( bdd_options );
}

bool show_store_entry<bdd_function_t>::operator()( bdd_function_t& bdd,
                                                   const std::string& dotname,
                                                   const cli_options& opts,
                                                   const properties::ptr& settings )
{
  using namespace std::placeholders;

  auto * fd = fopen( dotname.c_str(), "w" );

  if ( opts.vm.count( "add" ) )
  {
    std::vector<ADD> adds( bdd.second.size() );
    boost::transform( bdd.second, adds.begin(), std::bind( &BDD::Add, _1 ) );
    bdd.first.DumpDot( adds, 0, 0, fd );
  }
  else
  {
    bdd.first.DumpDot( bdd.second, 0, 0, fd );
  }

  fclose( fd );

  return true;
}

command_log_opt_t show_store_entry<bdd_function_t>::log() const
{
  return boost::none;
}

template<>
void print_store_entry_statistics<bdd_function_t>( std::ostream& os, const bdd_function_t& bdd )
{
  std::vector<double> minterms;

  for ( const auto& f : bdd.second )
  {
    minterms.push_back( f.CountMinterm( bdd.first.ReadSize() ) );
  }

  os << "[i] no. of variables: " << bdd.first.ReadSize() << std::endl
     << "[i] no. of nodes:     " << bdd.first.ReadKeys() << std::endl
     << "[i] no. of minterms:  " << any_join( minterms, " " ) << std::endl
     << "[i] level sizes:      " << any_join( level_sizes( bdd.first, bdd.second ), " " ) << std::endl
     << "[i] maximum fanout:   " << maximum_fanout( bdd.first, bdd.second ) << std::endl
     << "[i] complement edges: " << count_complement_edges( bdd.first, bdd.second ) << std::endl;


  for ( const auto& p : index( bdd.second ) )
  {
    os << "[i] info for output " << p.index << ":" << std::endl
       << "[i] - path count:               " << p.value.CountPath() << std::endl
       << "[i] - path count (to non-zero): " << Cudd_CountPathsToNonZero( p.value.getNode() ) << std::endl;
  }

  bdd.first.info();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
