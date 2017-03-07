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

  auto total = 0u, covered = 0u;

  for ( const auto& bundle : meta.bundles )
  {
    total += bundle.literals.size();
  }
  for ( const auto& bundle : covered_bundles )
  {
    covered += bundle.second.size();
  }

  //unsigned total   = boost::accumulate( meta.bundles | transformed( []( const aigmeta_bundle& bundle ) { return bundle.literals.size(); } ), 0u );
  //unsigned covered = boost::accumulate( covered_bundles | map_values | transformed( []( const std::list<unsigned>& l ) { return l.size(); } ), 0u );

  return (double)covered / total;
}

void aigmeta_coverage::print_report() const
{
  std::cout << "Coverage: " << boost::format( "%.2f" ) % coverage() << std::endl;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
