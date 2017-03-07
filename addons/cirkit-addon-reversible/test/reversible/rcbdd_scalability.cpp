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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE rcbdd_scalability

#include <iostream>
#include <list>

#include <boost/format.hpp>
#include <boost/range/counting_range.hpp>
#define timer timer_class
#include <boost/test/unit_test.hpp>
#undef timer

#include <core/properties.hpp>
#include <core/utils/benchmark_table.hpp>
#include <core/utils/timer.hpp>

#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/synthesis/rcbdd_synthesis.hpp>

using namespace cirkit;

void create_and_synthesize( unsigned n, double& runtime, std::function<void(BDD&, const rcbdd&)> func )
{
  reference_timer t( &runtime );

  rcbdd cf;
  cf.initialize_manager();
  cf.create_variables( n );

  BDD chi = cf.manager().bddOne();

  func( chi, cf );

  cf.set_chi( chi );

  circuit circ;
  properties::ptr settings( new properties );
  settings->set( "create_gates", false );
  //settings->set( "verbose", true );
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
      for ( unsigned i = 0u; i < n; ++i )
      {
        chi &= cf.x(i) ^ cf.y(i);
      }
    });
}

void synthesize_rotate( unsigned n, unsigned k, double& runtime )
{
  create_and_synthesize( n, runtime, [&n, &k]( BDD& chi, const rcbdd& cf ) {
      for ( unsigned i = 0u; i < n; ++i )
      {
        chi &= cf.y(i).Xnor( cf.x((i + k) % n));
      }
    });
}

void synthesize_invert_or_rotate( unsigned n, double& runtime )
{
  create_and_synthesize( 2u * n, runtime, [&n]( BDD& chi, const rcbdd& cf ) {
      for ( unsigned i = 0u; i < 2u * n; ++i )
      {
        if ( i % 2 == 0u )
        {
          chi &= cf.y(i).Xnor( cf.x((i + 2) % (2u * n)) );
        }
        else
        {
          chi &= cf.y(i) ^ cf.x(i);
        }
      }
    });
}

void synthesize_bitwise_xor( unsigned n, double& runtime )
{
  create_and_synthesize( 2u * n, runtime, [&n]( BDD& chi, const rcbdd& cf ) {
      for ( unsigned i = 0u; i < n; ++i )
      {
        chi &= cf.y(i).Xnor( cf.x(i) );
        chi &= cf.y(n + i).Xnor( cf.x(i) ^ cf.x(n + i) );
      }
    });
}

BOOST_AUTO_TEST_CASE(simple)
{
  double runtime;

  using exp_func_t  = std::function<void(unsigned, double&)>;
  using exp_tuple_t = std::tuple<unsigned, unsigned, std::string, exp_func_t>;
  std::vector<exp_tuple_t> experiments =
    { std::make_tuple( 1u, 150u, std::string( "identity" ),         exp_func_t( synthesize_identity         ) ),
      std::make_tuple( 1u, 150u, std::string( "invert" ),           exp_func_t( synthesize_inverter         ) ),
      std::make_tuple( 1u,  30u, std::string( "invert-or-rotate" ), exp_func_t( synthesize_invert_or_rotate ) ),
      std::make_tuple( 1u,  30u, std::string( "rotate k=3" ),       exp_func_t( []( unsigned n, double& runtime ) { return synthesize_rotate( n, 3u, runtime ); } ) ),
      std::make_tuple( 1u,  30u, std::string( "rotate k=5" ),       exp_func_t( []( unsigned n, double& runtime ) { return synthesize_rotate( n, 5u, runtime ); } ) ),
      std::make_tuple( 1u,  25u, std::string( "rotate k=7" ),       exp_func_t( []( unsigned n, double& runtime ) { return synthesize_rotate( n, 7u, runtime ); } ) ),
      std::make_tuple( 1u,  15u, std::string( "bitwise-xor" ),      exp_func_t( synthesize_bitwise_xor      ) ) };

  for ( const auto& exp : experiments )
  {
    const auto& from = std::get<0>( exp );
    const auto& to   = std::get<1>( exp );
    const auto& name = std::get<2>( exp );
    const auto& func = std::get<3>( exp );

    benchmark_table<unsigned, double> table( {"n", "Run-time"} );

    std::cout << "Experiment: " << name << std::endl;
    for ( auto i : boost::counting_range( from, to ) )
    {
      func( i, runtime );
      table.add( i, runtime );
      std::cout << "[I] " << i << ": " << runtime << std::endl;
    }
    table.print();
  }
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
