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
#define BOOST_TEST_MODULE restricted_growth_sequence

#include <iostream>
#include <list>
#include <map>

#include <boost/format.hpp>
#define timer timer_class
#include <boost/test/unit_test.hpp>
#undef timer

#include <core/utils/benchmark_table.hpp>
#include <core/utils/timer.hpp>

#include <reversible/rcbdd.hpp>

void restricted_growth_sequence( unsigned p, double& runtime )
{
  using namespace cirkit;

  //reference_timer t( &runtime );

  unsigned i = 0u;
  std::map<std::pair<unsigned,unsigned>,unsigned> a;

  for ( int k = p; k; --k )
  {
    for ( unsigned j = 0u; j < k; ++j )
    {
      a[{k,j}] = i++;
    }
  }

  unsigned n = i;

  rcbdd cf;
  cf.initialize_manager();
  cf.create_variables( n + 1u, false );
  cf.set_num_inputs( n );
  cf.set_num_outputs( 1u );

  BDD tmp1, tmp2;

  std::vector<BDD> fs( p + 1u, cf.manager().bddOne() );
  for ( int k = p; k; --k )
  {
    for ( unsigned j = 1u; j < k; ++j )
    {
      BDD x = cf.x( a[{k,0u}] );
      fs[0] = x.Ite( fs[j], cf.manager().bddZero() );
      tmp1 = x.Ite( cf.manager().bddZero(), fs[j + 1u] );
      tmp2 = x.Ite( cf.manager().bddZero(), fs[j] );
      for ( i = 1u; i < j; ++i )
      {
        BDD x2 = cf.x( a[{k,i}] );
        fs[0] = x2.Ite( tmp2, fs[0] );
        tmp1 = x2.Ite( cf.manager().bddZero(), tmp1 );
        tmp2 = x2.Ite( cf.manager().bddZero(), tmp2 );
      }
      fs[0] = cf.x( a[{k,j}] ).Ite( tmp1, fs[0] );
      for ( i++; i < k; ++i )
      {
        fs[0] = cf.x( a[{k,i}] ).Ite( cf.manager().bddZero(), fs[0] );
      }
      fs[j] = fs[0];
    }
  }

  BDD f = cf.x( a[{1,0}] ).Ite( fs[1u], cf.manager().bddZero() );

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

  benchmark_table<unsigned, unsigned, unsigned, double> table( { "p", "n", "m", "Run-time" }, true );
  double runtime;

  for ( unsigned i = 1; i < 8u; ++i )
  {
    unsigned p = i * 5u;
    restricted_growth_sequence( p, runtime );
    table.add( p, ((p * (p + 1u)) / 2u), 1u, runtime );
  }

  table.print();
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
