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

#include "bdd.hpp"

#include <iostream>
#include <vector>

#include <boost/lexical_cast.hpp>

#include <cuddObj.hh>

#include <alice/rules.hpp>

#include <core/cli/stores.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/string_utils.hpp>

using namespace boost::program_options;

namespace cirkit
{

bdd_command::bdd_command( const environment::ptr& env )
  : cirkit_command( env, "BDD manipulation" )
{
  opts.add_options()
    ( "characteristic,c", value( &characteristic ), "Compute characteristic function (x: inputs first, y: outputs first)" )
    ( "clique",           value( &clique ),         "Computes clique(n,k) function, give n,k as string" )
    ( "new,n",                                      "Add a new entry to the store; if not set, the current entry is overriden" )
    ;
}

command::rules_t bdd_command::validity_rules() const
{
  return { has_store_element_if_set<bdd_function_t>( *this, env, "characteristic" ) };
}

bool bdd_command::execute()
{
  auto& bdds = env->store<bdd_function_t>();

  if ( is_set( "characteristic" ) )
  {
    auto bdd = bdds.current();

    extend_if_new( bdds );

    bdds.current() = compute_characteristic( bdd, characteristic == "x" );
  }

  else if ( is_set( "clique" ) )
  {
    const auto nk = split_string_pair( clique, "," );
    const auto n = boost::lexical_cast<unsigned>( nk.first );
    const auto k = boost::lexical_cast<unsigned>( nk.second );

    Cudd mgr;
    auto func = mgr.bddZero();

    lexicographic_combinations( n, k, [&mgr, &func, k]( const std::vector<unsigned>& v ) {
        auto term = mgr.bddOne();
        lexicographic_combinations( k, 2u, [&mgr, &term, &v]( const std::vector<unsigned>& idxs ) {
            const auto v0 = v[idxs[0u]];
            const auto v1 = v[idxs[1u]];
            const auto edge_idx = v0 + ( v1 * ( v1 - 1 ) ) / 2;

            term &= mgr.bddVar( edge_idx );

            return false;
          } );

        func |= term;

        return false;
      } );

    extend_if_new( bdds );
    bdds.current() = {mgr, {func}};
  }

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
