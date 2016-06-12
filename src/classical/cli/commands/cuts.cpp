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

#include "cuts.hpp"

#include <iostream>

#include <boost/format.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/program_options.hpp>
#include <core/utils/range_utils.hpp>
#include <classical/functions/cuts/paged.hpp>
#include <classical/mig/mig_cuts_paged.hpp>
#include <classical/functions/cuts/traits.hpp>
#include <classical/utils/cut_enumeration.hpp>
#include <classical/utils/truth_table_utils.hpp>

using namespace boost::program_options;
using boost::format;

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

cuts_command::cuts_command( const environment::ptr& env ) : aig_mig_command( env, "Computes cuts of an AIG", "Enumerate cuts for %s" )
{
  opts.add_options()
    ( "node_count,k", value_with_default( &node_count ), "Number of nodes in a cut" )
    ( "truthtable,t",                                    "Prints truth tables when verbose" )
    ( "cone_count,c",                                    "Prints nodes in cut cone when verbose" )
    ( "depth,d",                                         "Prints depth of cut when verbose " )
    ( "parallel",                                        "Parallel cut enumeration for AIGs" )
    ;
  be_verbose();
}

bool cuts_command::execute_aig()
{
  paged_aig_cuts cuts( aig(), node_count, is_set( "parallel" ) );
  std::cout << boost::format( "[i] found %d cuts in %.2f secs (%d KB)" ) % cuts.total_cut_count() % cuts.enumeration_time() % ( cuts.memory() >> 10u ) << std::endl;

  if ( is_verbose() )
  {
    for ( const auto& p : boost::make_iterator_range( vertices( aig() ) ) )
    {
      std::cout << boost::format( "[i] node %d has %d cuts" ) % p % cuts.count( p ) << std::endl;
      for ( const auto& cut : cuts.cuts( p ) )
      {
        std::cout << "[i] - {" << any_join( cut.range(), ", " ) << "}";

        if ( is_set( "cone_count" ) )
        {
          std::cout << format( " (size: %d)" ) % cut.size();
        }

        if ( is_set( "truthtable" ) )
        {
          std::cout << " " << tt_to_hex( cuts.simulate( p, cut ) );
        }

        std::cout << std::endl;
      }
    }
  }

  return true;
}

bool cuts_command::execute_mig()
{
  mig_cuts_paged cuts( mig(), node_count );
  std::cout << boost::format( "[i] found %d cuts in %.2f secs (%d KB)" ) % cuts.total_cut_count() % cuts.enumeration_time() % ( cuts.memory() >> 10u ) << std::endl;

  if ( is_verbose() )
  {
    for ( const auto& p : boost::make_iterator_range( vertices( mig() ) ) )
    {
      std::cout << boost::format( "[i] node %d has %d cuts" ) % p % cuts.count( p ) << std::endl;
      for ( const auto& cut : cuts.cuts( p ) )
      {
        std::cout << "[i] - {" << any_join( cut.range(), ", " ) << "}";

        if ( is_set( "cone_count" ) )
        {
          std::cout << format( " (size: %d)" ) % cuts.size( p, cut );
        }

        if ( is_set( "depth" ) )
        {
          std::cout << format( " (depth: %d)" ) % cuts.depth( p, cut );
        }

        if ( is_set( "truthtable" ) )
        {
          std::cout << " " << tt_to_hex( cuts.simulate( p, cut ) );
        }

        std::cout << std::endl;
      }
    }
  }

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
