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
#define BOOST_TEST_MODULE redundancy_functions

#include <iostream>
#include <list>

#include <boost/format.hpp>
#include <boost/test/unit_test.hpp>

#include <core/utils/benchmark_table.hpp>
#include <core/utils/timer.hpp>

#include <reversible/rcbdd.hpp>

void two_level_redundancy_function( unsigned p, unsigned q, double& runtime )
{
  using namespace cirkit;

  reference_timer t( &runtime );

  unsigned n = p + p * q;

  rcbdd cf;
  cf.initialize_manager();
  cf.create_variables( n + 1u, false );
  cf.set_num_inputs( n );
  cf.set_num_outputs( 1u );

  BDD f = cf.manager().bddOne();

  for ( unsigned j = 0u; j < q; ++j )
  {
    BDD inner = cf.manager().bddZero();
    for ( unsigned i = 0u; i < p; ++i )
    {
      inner |= ( cf.x( 1u + i ) & cf.x( 1u + p + i * p + j ) );
    }
    f &= inner;
  }

  /* Embed */
  BDD chi = cf.manager().bddOne();

  chi &= !cf.y( 0u ) ^ ( cf.x( 0u ) ^ f );

  for ( unsigned i = 1u; i <= n; ++i )
  {
    chi &= !cf.x( i ) ^ cf.y( i );
  }

  cf.set_chi( chi );
}

BOOST_AUTO_TEST_CASE(simple)
{
  using namespace cirkit;

  std::vector<std::pair<unsigned, unsigned>> numbers{ {5u, 5u}, {6u, 6u}, {7u, 7u}, {8u, 8u}, {9u, 9u}, {10u, 10u}, {11u, 10u}, {10u, 11u}, {11u, 11u} };

  benchmark_table<unsigned, unsigned, unsigned, unsigned, double> table( { "p", "q", "n", "m", "Run-time" }, true );
  unsigned p, q;
  double runtime;

  for ( const auto& e : numbers )
  {
    std::tie( p, q ) = e;
    two_level_redundancy_function( p, q, runtime );
    table.add( p, q, p + p * q, 1u, runtime );
  }

  table.print();
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
