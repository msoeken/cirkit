/* RevKit (www.revkit.org)
 * Copyright (C) 2009-2014  University of Bremen
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
#define BOOST_TEST_MODULE extend_truth_table

#include <fstream>

#include <boost/test/unit_test.hpp>

#include <core/io/read_pla_to_bdd.hpp>

#include <reversible/truth_table.hpp>
#include <reversible/functions/approximate_additional_lines.hpp>
#include <reversible/functions/extend_pla.hpp>
#include <reversible/io/read_pla.hpp>
#include <reversible/io/write_pla.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
  using boost::unit_test::framework::master_test_suite;
  using namespace cirkit;

  if ( master_test_suite().argc != 2u ) return;

  binary_truth_table pla, extended;
  read_pla_settings settings;
  settings.extend = false;
  read_pla( pla, master_test_suite().argv[1], settings );
  extend_pla( pla, extended );

  write_pla( extended, "/tmp/test.pla" );
  unsigned lines = approximate_additional_lines( "/tmp/test.pla" );

  std::filebuf fb;
  fb.open( "/tmp/lines_extend_truth_table", std::ios::out );

  std::ostream os( &fb );
  os << lines << std::endl;
  fb.close();
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
