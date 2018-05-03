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

#include "gates.hpp"

#include <vector>

#include <alice/rules.hpp>
#include <cli/reversible_stores.hpp>
#include <reversible/pauli_tags.hpp>
#include <reversible/target_tags.hpp>

#include <fmt/format.h>

namespace cirkit
{

gates_command::gates_command( const environment::ptr& env )
  : cirkit_command( env, "Prints gates distribution" )
{
}

command::rules gates_command::validity_rules() const
{
  return {has_store_element<circuit>( env )};
}

void gates_command::execute()
{
  const auto& circuits = env->store<circuit>();
  const auto& circ = circuits.current();

  std::vector<std::vector<unsigned>> gate_controls( 6u );
  std::vector<std::string> label{"Toffoli", "Fredkin", "Single-target", "Pauli roots", "Hadamard", "Other"};
  auto max_controls = 0u;

  for ( const auto& g : circ )
  {
    auto key = gate_controls.size() - 1u;

    /* key */
    if ( is_toffoli( g ) )
    {
      key = 0u;
    }
    else if ( is_fredkin( g ) )
    {
      key = 1u;
    }
    else if ( is_stg( g ) )
    {
      key = 2u;
    }
    else if ( is_pauli( g ) )
    {
      key = 3u;
    }
    else if ( is_hadamard( g ) )
    {
      key = 4u;
    }

    auto& v = gate_controls[key];
    const auto num_controls = g.controls().size();
    if ( v.size() <= num_controls )
    {
      v.resize( num_controls + 1u );
    }
    v[num_controls]++;

    if ( max_controls < num_controls )
    {
      max_controls = num_controls;
    }
  }

  std::cout << "     controls";
  for ( auto i = 0u; i <= max_controls; ++i )
  {
    std::cout << fmt::format( " | {:>4}", i );
  }
  std::cout << " | total" << std::endl;

  for ( auto i = 0u; i < gate_controls.size(); ++i )
  {
    std::cout << label[i] << std::string( 13u - label[i].size(), ' ' );
    gate_controls[i].resize( max_controls + 1u );
    auto total = 0u;

    for ( auto c : gate_controls[i] )
    {
      std::cout << fmt::format( " | {:>4}", c );
      total += c;
    }
    std::cout << fmt::format( " | {:>5}", total ) << std::endl;
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
