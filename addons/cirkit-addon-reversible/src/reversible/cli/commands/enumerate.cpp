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

#include "enumerate.hpp"

#include <boost/dynamic_bitset.hpp>
#include <boost/format.hpp>

#include <core/utils/bitset_utils.hpp>
#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/synthesis/transformation_based_synthesis.hpp>

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

enumerate_command::enumerate_command( const environment::ptr& env )
  : command( env, "Different enumeration routines" )
{
  opts.add_options()
    ( "four_stg_pqcs", "Enumerate all four variable single-target gates for pQCS" )
    ;
}

bool enumerate_command::execute()
{
  if ( opts.is_set( "four_stg_pqcs" ) )
  {
    const auto n = 3u; /* number of lines - 1 */

    boost::dynamic_bitset<> bs( 1u << n );

    inc( bs ); /* skip empty circuit */

    do {
      /* create function */
      binary_truth_table spec;

      boost::dynamic_bitset<> ibs( n );

      do {
        const auto val = ibs.to_ulong();

        binary_truth_table::cube_type in, out;

        for ( auto i = 0u; i < n; ++i )
        {
          in.push_back( ibs.test( i ) );
          out.push_back( ibs.test( i ) );
        }

        auto in_0 = in;
        auto out_0 = out;
        in_0.push_back( false );
        out_0.push_back( bs.test( val ) );
        in.push_back( true );
        out.push_back( !bs.test( val ) );

        spec.add_entry( in_0, out_0 );
        spec.add_entry( in, out );

        inc( ibs );
      } while ( ibs.any() );

      /* create circuit */
      circuit circ;
      transformation_based_synthesis( circ, spec );

      /* create string */
      std::vector<std::string> circ_str;

      for ( const auto& g : circ )
      {
        std::string gate_str( n + 1, ' ' );

        const auto t = g.targets().front();
        gate_str[t] = 'X';

        for ( const auto& c : g.controls() )
        {
          gate_str[c.line()] = '1' + t;
        }

        circ_str.push_back( gate_str );
      }

      /* print circuit */
      std::cout << "STG-4-" << bs << std::endl;
      for ( auto l = 0u; l < n + 1; ++l )
      {
        for ( auto g = 0u; g < circ_str.size(); ++g )
        {
          const auto chr = circ_str[g][l];
          if ( chr == ' ' )
          {
            std::cout << " I   ";
          }
          else if ( chr == 'X' )
          {
            std::cout << " X   ";
          }
          else
          {
            std::cout << boost::format( "C(%c) " ) % chr;
          }
        }
        std::cout << std::endl;
      }

      std::cout << std::endl;

      inc( bs );
    } while ( bs.any() );
  }

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
