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

#include "add_stg.hpp"

#include <numeric>

#include <boost/pending/integer_log2.hpp>

#include <core/utils/range_utils.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/utils/permutation.hpp>

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

void add_stg_as_other( circuit& circ, const tt& func, const tt& func_real, const std::vector<unsigned>& line_map )
{
  const auto num_vars = tt_num_vars( func );

  std::vector<kitty::detail::spectral_operation> trans;
  const auto func_norm = kitty::exact_spectral_canonization( to_kitty( func ),
                                                             [&trans]( const std::vector<kitty::detail::spectral_operation>& ops ) {
                                                               std::copy( ops.begin(), ops.end(), std::back_inserter( trans ) );
                                                             } );
  const auto func_real_norm = kitty::exact_spectral_canonization( to_kitty( func_real ),
                                                                  [&trans]( const std::vector<kitty::detail::spectral_operation>& ops ) {
                                                                    std::copy( ops.rbegin(), ops.rend(), std::back_inserter( trans ) );
                                                                  } );

  assert( func_norm == func_real_norm );

  std::vector<unsigned> line( num_vars + 1u );
  if ( !line_map.empty() )
  {
    std::copy( line_map.begin(), line_map.end(), line.begin() );
  }
  else
  {
    std::iota( line.begin(), line.end(), 0u );
  }

  auto index = circ.num_gates();

  /* we defer the adding of NOT gates to the end, but NOT gates are just stored
     and used to adjust the polarities of the controls.
     We also try to merge an output inversion into the control line of a disjoint
     transformation. */
  boost::dynamic_bitset<> not_mask( num_vars + 1 );

  /* we also defer adding SWAP gates to the end.  The swaps are stored in a permutation
     that is used when other gates are added.  Finally the SWAP gates are computed by
     decomposing the permutation into transpositions. */
  permutation_t perm = identity_permutation( num_vars );

  for ( const auto& t : trans )
  {
    switch ( t._kind )
    {
    default:
      assert( false );
    case kitty::detail::spectral_operation::kind::permutation:
      std::swap( perm[boost::integer_log2( t._var1 )], perm[boost::integer_log2( t._var2 )] );
      break;
    case kitty::detail::spectral_operation::kind::input_negation:
      not_mask.flip( perm[boost::integer_log2( t._var1 )] );
      break;
    case kitty::detail::spectral_operation::kind::output_negation:
      not_mask.flip( num_vars );
      break;
    case kitty::detail::spectral_operation::kind::spectral_translation:
    {
      const auto v1 = perm[boost::integer_log2( t._var1 )];
      const auto v2 = perm[boost::integer_log2( t._var2 )];
      insert_cnot( circ, index, make_var( line[v2], !not_mask.test( v2 ) ), line[v1] );
      insert_cnot( circ, index, make_var( line[v2], !not_mask.test( v2 ) ), line[v1] );
      ++index;
    }
    break;
    case kitty::detail::spectral_operation::kind::disjoint_translation:
    { /* disjoint transformation of output */
      const auto v1 = perm[boost::integer_log2( t._var1 )];

      auto pol = !not_mask.test( v1 );
      if ( not_mask.test( num_vars ) )
      {
        not_mask.reset( num_vars );
        pol = !pol;
      }
      insert_cnot( circ, index, make_var( line[v1], pol ), line[num_vars] );
    }
    break;
    }
  }

  /* add SWAPS */
  const auto perm_inv = permutation_invert( perm );
  for ( const auto& trans : permutation_to_transpositions( perm_inv ) )
  {
    insert_fredkin( circ, index, {}, line[trans.first], line[trans.second] );
    insert_fredkin( circ, index, {}, line[trans.first], line[trans.second] );
    ++index;
  }

  /* add inverters from mask */
  for ( auto i = 0u; i < num_vars; ++i )
  {
    if ( not_mask.test( i ) )
    {
      insert_not( circ, index, line[perm_inv[i]] );
      insert_not( circ, index, line[perm_inv[i]] );
      ++index;
    }
  }
  if ( not_mask.test( num_vars ) )
  {
    insert_not( circ, index, line[num_vars] );
  }

  /* add middle gate */
  auto& g = circ.insert_gate( index );
  g.set_type( stg_tag( func_real ) );
  for ( auto i = 0u; i < num_vars; ++i )
  {
    g.add_control( make_var( line[i], true ) );
  }
  g.add_target( line[num_vars] );
}
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
