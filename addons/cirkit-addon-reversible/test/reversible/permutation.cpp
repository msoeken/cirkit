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
#define BOOST_TEST_MODULE permutation

#define timer timer_class
#include <boost/test/unit_test.hpp>
#undef timer

#include <core/utils/bdd_utils.hpp>
#include <core/utils/range_utils.hpp>

#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/synthesis/exact_synthesis.cpp>
#include <reversible/utils/foreach_vshape.hpp>
#include <reversible/utils/permutation.hpp>

using namespace cirkit;

BOOST_AUTO_TEST_CASE(simple)
{
  /* number of lines */
  auto n = 3u;

  /* for RCBDDs */
  rcbdd cf;
  cf.initialize_manager();
  cf.create_variables( n );

  /* statistics */
  auto total    = 0u;
  auto selfinv  = 0u;
  auto selfdual = 0u;
  auto monotone = 0u;
  auto unate    = 0u;
  auto horn     = 0u;

  std::vector<int> ps; /* for unateness test */

  foreach_vshape( n, [&]( const circuit& circ ) {
      permutation_t perm = circuit_to_permutation( circ );
      BDD func = cf.create_from_circuit( circ );

      ++total;
      if ( cf.is_self_inverse( func ) )         { ++selfinv;  }
      if ( is_selfdual( cf.manager(), func ) )  { ++selfdual; }
      if ( is_monotone( cf.manager(), func ) )  { ++monotone; }
      if ( is_unate( cf.manager(), func, ps ) ) { ++unate;    }
      if ( is_horn( cf.manager(), func ) )      { ++horn;     }
    });

  std::cout << "[i] circuits investigated: " << total << std::endl
            << "[i] - self inverse:        " << selfinv << std::endl
            << "[i] - self dual:           " << selfdual << std::endl
            << "[i] - monotone:            " << monotone << std::endl
            << "[i] - unate:               " << unate << std::endl
            << "[i] - horn:                " << horn << std::endl;

  permutation_t perm = {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u};

  std::vector<unsigned> sizes( 9u, 0u );

  do
  {
    if ( is_simple( perm ) )
    {
        std::cout << "[i] simple: " << permutation_to_string( perm ) << std::endl;

        binary_truth_table spec;

        for ( auto i = 0u; i < 8u; ++i )
        {
          spec.add_entry( number_to_truth_table_cube( i, 3u ), number_to_truth_table_cube( perm.at( i ), 3u ) );
        }

        circuit circ;
        auto es_settings = std::make_shared<properties>();
        es_settings->set( "negative", false );
        exact_synthesis( circ, spec, es_settings );

        std::cout << "[i] " << circ.num_gates() << std::endl;

        sizes[circ.num_gates()]++;
    }
  } while ( std::next_permutation( perm.begin(), perm.end() ) );

  std::cout << "[i] sizes: " << any_join( sizes, " " ) << std::endl;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
