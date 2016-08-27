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

#include "dectest.hpp"

#include <iostream>

#include <boost/program_options.hpp>

#include <alice/rules.hpp>

#include <range/v3/range_for.hpp>
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/view/split.hpp>
#include <range/v3/view/transform.hpp>

#include <classical/cli/stores.hpp>
#include <classical/utils/truth_table_utils.hpp>

using namespace ranges::v3;
using boost::program_options::value;

namespace cirkit
{

unsigned tt_is_decomposable( const tt& func, const tt& mask, unsigned var )
{
  tt fcopy = func & mask;
  tt fvarp = tt_nth_var( var ) & mask;
  tt fvarn = ~tt_nth_var( var ) & mask;
  tt_align( fcopy, fvarp );
  tt_align( fcopy, fvarn );

  if ( fcopy.is_subset_of( fvarp ) ) return 1;
  if ( fcopy.is_subset_of( fvarn ) ) return 2;
  if ( fvarp.is_subset_of( fcopy ) ) return 3;
  if ( fvarn.is_subset_of( fcopy ) ) return 4;

  return 0;
}

dectest_command::dectest_command( const environment::ptr& env )
  : cirkit_command( env, "Checks truth tables for decomposability" )
{
  opts.add_options()
    ( "staircase,s", value( &staircase ), "comma-separated list of ids: i,j,k,...\nchecks for decompositions f = g1(xi, g2(xj, g3(xk, others)))" )
    ;
}

command::rules_t dectest_command::validity_rules() const
{
  return {has_store_element<tt>( env )};
}

bool dectest_command::execute()
{
  /* truth table in store */
  const auto& func = env->store<tt>().current();

  if ( is_set( "staircase" ) )
  {
    std::cout << "f  = " << tt_to_hex( func ) << std::endl;

    auto mask = ~tt( func.size() );

    /* split ids into list of ints */
    RANGES_FOR( auto i, staircase
                | view::split( ',' )
                | view::transform( []( const std::string& s ) { return boost::lexical_cast<unsigned>( s ); } ) )
    {
      std::cout << " f  = " << func << std::endl;
      std::cout << " m  = " << mask << std::endl;

      const auto fvar = tt_nth_var( i );
      std::cout << " x" << i << " = " <<  fvar << std::endl;
      std::cout << "!x" << i << " = " << ~fvar << std::endl;

      const auto d = tt_is_decomposable( func, mask, i );
      if ( !d )
      {
        std::cout << "decomposition failed at variable " << i << std::endl;
        break;
      }
      else
      {
        std::cout << d << std::endl;
        switch ( d )
        {
        case 1: /* AND(x, g) */
        case 4: /* OR(!x, g) */
          mask &= fvar;
          break;
        case 2: /* AND(!x, g) */
        case 3: /* OR(x, g) */
          mask &= ~fvar;
          break;
        default: /* impossible */
          assert( 0 );
          break;
        }
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
