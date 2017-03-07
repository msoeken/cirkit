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
#define BOOST_TEST_MODULE truth_table

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include <reversible/truth_table.hpp>
#include <reversible/utils/truth_table_helpers.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
  using boost::test_tools::output_test_stream;

  using namespace cirkit;

  binary_truth_table spec;

  spec.add_entry( number_to_truth_table_cube( 0u, 2u ), number_to_truth_table_cube( 0u, 2u ) );
  spec.add_entry( number_to_truth_table_cube( 1u, 2u ), number_to_truth_table_cube( 1u, 2u ) );
  spec.add_entry( number_to_truth_table_cube( 2u, 2u ), number_to_truth_table_cube( 3u, 2u ) );
  spec.add_entry( number_to_truth_table_cube( 3u, 2u ), number_to_truth_table_cube( 2u, 2u ) );

  output_test_stream output;
  output << spec;

  BOOST_CHECK( !output.is_empty( false ) );
  BOOST_CHECK( output.is_equal( "00 00\n01 01\n10 11\n11 10\n" ) );

  bitset_vector_t tt_vec = truth_table_to_bitset_vector( spec );
  BOOST_CHECK( tt_vec.size() == 4u );
  BOOST_CHECK( tt_vec[0u] == boost::dynamic_bitset<>( 2u, 0u ) );
  BOOST_CHECK( tt_vec[1u] == boost::dynamic_bitset<>( 2u, 1u ) );
  BOOST_CHECK( tt_vec[2u] == boost::dynamic_bitset<>( 2u, 3u ) );
  BOOST_CHECK( tt_vec[3u] == boost::dynamic_bitset<>( 2u, 2u ) );

  for ( unsigned i = 0u; i < 4u; ++i )
  {
    std::cout << boost::dynamic_bitset<>( 2u, i ) << " <-> " << tt_vec[i] << std::endl;
  }
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
