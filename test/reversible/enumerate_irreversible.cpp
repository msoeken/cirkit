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
#define BOOST_TEST_MODULE enumerate_irreversible

#include <iostream>

#include <boost/format.hpp>
#include <boost/test/unit_test.hpp>

#include <reversible/io/print_circuit.hpp>
#include <reversible/synthesis/exact_synthesis.hpp>
#include <reversible/utils/foreach_function.hpp>

using namespace cirkit;

BOOST_AUTO_TEST_CASE(simple)
{
  using boost::format;

  auto n     = 3u; auto total = ( 1u << ( 1u << n ) );
  auto index = 0u;

  auto settings = std::make_shared<properties>();
  settings->set( "negative", true );
  auto statistics = std::make_shared<properties>();

  foreach_function_as_truth_table( n, [&]( const binary_truth_table& spec ) {

      std::cout << format( "[i] circuit %d/%d" ) % ++index % total << std::endl;

      circuit circ;
      exact_synthesis( circ, spec, settings, statistics );

      std::cout << circ;
      std::cout << format( "[i] runtime: %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl << std::endl;

    } );

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
