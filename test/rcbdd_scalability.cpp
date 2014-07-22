/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2014  The RevKit Developers <revkit@informatik.uni-bremen.de>
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
#define BOOST_TEST_MODULE rcbdd_scalability

#include <iostream>
#include <list>

#include <boost/format.hpp>
#include <boost/range/irange.hpp>
#include <boost/test/unit_test.hpp>

#include <core/properties.hpp>
#include <core/utils/benchmark_table.hpp>
#include <core/utils/timer.hpp>

#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/synthesis/rcbdd_synthesis.hpp>

using namespace cirkit;

void create_and_synthesize( unsigned n, double& runtime, std::function<void(BDD&, const rcbdd&)> func )
{
  reference_timer rt( &runtime );
  timer<reference_timer> t( rt );

  rcbdd cf;
  cf.initialize_manager();
  cf.create_variables( n );

  BDD chi = cf.manager().bddOne();

  func( chi, cf );

  cf.set_chi( chi );

  circuit circ;
  properties::ptr settings( new properties );
  settings->set( "create_gates", false );
  rcbdd_synthesis( circ, cf, settings );
}

void synthesize_identity( unsigned n, double& runtime )
{
  create_and_synthesize( n, runtime, [&n]( BDD& chi, const rcbdd& cf ) {
      for ( unsigned i = 0u; i < n; ++i )
      {
        chi &= cf.x(i).Xnor( cf.y(i) );
      }
    });
}

void synthesize_inverter( unsigned n, double& runtime )
{
  create_and_synthesize( n, runtime, [&n]( BDD& chi, const rcbdd& cf ) {
      for (unsigned i = 0u; i < n; ++i)
      {
        chi &= cf.x(i) ^ cf.y(i);
      }
    });
}

void synthesize_bitwise_and( unsigned n, double& runtime )
{
  create_and_synthesize( 2u * n, runtime, [&n]( BDD& chi, const rcbdd& cf ) {
      for (unsigned i = 0u; i < n; ++i)
      {
        chi &= cf.x(i).Xnor( cf.y(i) );
        chi &= cf.y(n + i).Xnor( cf.x(i) & cf.x(n + i) );
      }
    });
}

BOOST_AUTO_TEST_CASE(simple)
{
  double runtime;

  typedef std::function<void(unsigned, double&)> exp_func_t;
  typedef std::tuple<unsigned, unsigned, std::string, exp_func_t> exp_tuple_t;
  std::vector<exp_tuple_t> experiments =
    { std::make_tuple( 0u, 20u, std::string( "identity" ),    exp_func_t( synthesize_identity    ) ),
      std::make_tuple( 0u, 20u, std::string( "invert" ),      exp_func_t( synthesize_inverter    ) ),
      std::make_tuple( 0u,  2u, std::string( "bitwise-and" ), exp_func_t( synthesize_bitwise_and ) ) };

  for ( const auto& exp : experiments )
  {
    const unsigned& from    = std::get<0>( exp );
    const unsigned& to      = std::get<1>( exp );
    const std::string& name = std::get<2>( exp );
    const exp_func_t&  func = std::get<3>( exp );

    benchmark_table<unsigned, double> table( {"n", "Run-time" } );

    for ( unsigned i : boost::irange( from, to ) )
    {
      func( i, runtime );
      table.add( i, runtime );
    }

    std::cout << "Experiment: " << name << std::endl;
    table.print();
  }
}

// Local Variables:
// c-basic-offset: 2
// End:
