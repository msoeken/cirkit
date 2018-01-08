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

#include "perm.hpp"

#include <iostream>

#include <boost/dynamic_bitset.hpp>

#include <alice/rules.hpp>
#include <core/utils/range_utils.hpp>
#include <cli/reversible_stores.hpp>
#include <reversible/utils/truth_table_helpers.hpp>

namespace cirkit
{

perm_command::perm_command( const environment::ptr& env )
  : cirkit_command( env, "Prints permuation of a reversible truth table" )
{
  add_flag( "--one,-o", "start counting from 1" );
}

command::rules perm_command::validity_rules() const
{
  return {has_store_element<binary_truth_table>( env )};
}

void perm_command::execute()
{
  const auto& specs = env->store<binary_truth_table>();

  const auto bitsets = truth_table_to_bitset_vector( specs.current() );

  perm.clear();

  for ( const auto& bs : bitsets )
  {
    auto v = bs.to_ulong();
    if ( is_set( "one" ) ) ++v;
    perm.push_back( v );
  }

  std::cout << "[i] permutation: " << any_join( perm, " " ) << std::endl;
}

nlohmann::json perm_command::log() const
{
  return nlohmann::json({
      {"permutation", perm}
    });
}


}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
