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

#include "esopps.hpp"

#include <alice/rules.hpp>

#include <cli/stores.hpp>
#include <cli/reversible_stores.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <reversible/circuit.hpp>
#include <reversible/gate.hpp>
#include <reversible/pauli_tags.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/functions/add_gates.hpp>

#include <kitty/kitty.hpp>

namespace cirkit
{

esopps_command::esopps_command( const environment::ptr& env )
  : cirkit_command( env, "ESOP-based phase synthesis" )
{
  add_new_option();
}

command::rules esopps_command::validity_rules() const
{
  return {has_store_element<tt>( env )};
}

void esopps_command::execute()
{
  const auto& tts = env->store<tt>();

  auto tt = to_kitty( tts.current() );
  auto esop = kitty::esop_from_optimum_pkrm( tt );

  circuit circ( tt.num_vars() );

  for ( const auto& cube : esop )
  {
    std::vector<unsigned> negs;
    std::vector<unsigned> ctrls;

    auto bits = cube._bits;
    auto mask = cube._mask;

    for ( auto i = 0u; i < tt.num_vars(); ++i )
    {
      if ( mask & 1 )
      {
        ctrls.push_back( i );
        if ( !( bits & 1 ) )
        {
          negs.push_back( i );
        }
      }
      bits >>= 1;
      mask >>= 1;
    }

    assert( !ctrls.empty() );

    for ( auto n : negs )
    {
      append_not( circ, n );
    }
    auto& g = append_pauli( circ, ctrls.front(), pauli_axis::Z );
    for ( auto i = 1; i < ctrls.size(); ++i )
    {
      g.add_control( make_var( ctrls[i], true ) );
    }
    for ( auto n : negs )
    {
      append_not( circ, n );
    }
  }

  auto& circs = env->store<circuit>();
  extend_if_new( circs );
  circs.current() = circ;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
