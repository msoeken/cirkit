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

#include "circuit_to_aig.hpp"

#include <classical/functions/aig_from_truth_table.hpp>
#include <classical/utils/aig_utils.hpp>
#include <reversible/target_tags.hpp>

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
    if ( is_toffoli( g ) )
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
    else if ( is_fredkin( g ) )
    {
      const auto target1 = g.targets()[0u];
      const auto target2 = g.targets()[1u];

      if ( g.controls().empty() )
      {
        std::swap( fs[target1], fs[target2] );
      }
      else
      {
        std::vector<aig_function> operands;
        for ( const auto& c : g.controls() )
        {
          operands += make_function( fs[c.line()], !c.polarity() );
        }
        const auto cond = aig_create_nary_and( aig, operands );
        fs[target1] = aig_create_ite( aig, cond, fs[target2], fs[target1] );
        fs[target2] = aig_create_ite( aig, cond, fs[target1], fs[target2] );
      }
    }
    else if ( is_stg( g ) )
    {
      const auto& stg = boost::any_cast<stg_tag>( g.type() );
      std::vector<aig_function> operands;
      for ( const auto& c : g.controls() )
      {
        operands += make_function( fs[c.line()], false );
      }
      const auto target = g.targets().front();
      fs[target] = aig_create_xor( aig, fs[target], aig_from_truth_table_naive( aig, stg.function, operands ) );
    }
    else
    {
      assert( false );
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
