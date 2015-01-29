/* RevKit (www.revkit.org)
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
