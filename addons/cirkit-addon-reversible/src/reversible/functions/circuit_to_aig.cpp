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

#include "circuit_to_aig.hpp"

#include <classical/utils/aig_utils.hpp>

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

aig_graph circuit_to_aig( const circuit& circ )
{
  std::vector<aig_function> fs( circ.lines() );

  aig_graph aig;
  aig_initialize( aig );

  /* I/O vector */
  for ( auto i = 0u; i < circ.lines(); ++i )
  {
    const auto constant = circ.constants()[i];
    if ( constant != boost::none )
    {
      fs[i] = aig_get_constant( aig, *constant );
    }
    else
    {
      fs[i] = aig_create_pi( aig, circ.inputs()[i] );
    }
  }

  /* compute all gates */
  for ( const auto& g : circ )
  {
    const auto target = g.targets().front();

    if ( g.controls().empty() )
    {
      fs[target].complemented ^= 1;
    }
    else
    {
      std::vector<aig_function> operands;
      for ( const auto& c : g.controls() )
      {
        operands += make_function( fs[c.line()], !c.polarity() );
      }
      fs[target] = aig_create_xor( aig, fs[target], aig_create_nary_and( aig, operands ) );
    }
  }

  /* create outputs */
  for ( auto i = 0u; i < circ.lines(); ++i )
  {
    if ( circ.garbage()[i] ) { continue; }

    aig_create_po( aig, fs[i], circ.outputs()[i] );
  }

  return aig;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
