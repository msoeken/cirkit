/* CirKit: A circuit toolkit
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


#include "aigmeta_coverage.hpp"

#include <boost/assign/std/list.hpp>
#include <boost/format.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/numeric.hpp>

using namespace boost::assign;

namespace cirkit
{

void aigmeta_coverage::cover( const aigmeta_bundle& bundle, unsigned literal )
{
  covered_bundles[&bundle] += literal;
}

double aigmeta_coverage::coverage() const
{
  using boost::adaptors::transformed;
  using boost::adaptors::map_values;

  unsigned total   = boost::accumulate( meta.bundles | transformed( []( const aigmeta_bundle& bundle ) { return bundle.literals.size(); } ), 0u );
  unsigned covered = boost::accumulate( covered_bundles | map_values | transformed( []( const std::list<unsigned>& l ) { return l.size(); } ), 0u );

  return (double)covered / total;
}

void aigmeta_coverage::print_report() const
{
  std::cout << "Coverage: " << boost::format( "%.2f" ) % coverage() << std::endl;
}

}

// Local Variables:
// c-basic-offset: 2
// End:
