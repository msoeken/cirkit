/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE permutation

#include <boost/test/unit_test.hpp>

#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/utils/foreach_vshape.hpp>
#include <reversible/utils/permutation.hpp>

using namespace cirkit;

BOOST_AUTO_TEST_CASE(simple)
{
  /* for RCBDDs */
  rcbdd cf;
  cf.initialize_manager();
  cf.create_variables( 3u );

  foreach_vshape( 3u, [&cf]( const circuit& circ ) {
      permutation_t perm = circuit_to_permutation( circ );
      BDD func = cf.create_from_circuit( circ );

      std::cout << cf.is_self_inverse( func ) << std::endl;
      /* ... */
    });
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
