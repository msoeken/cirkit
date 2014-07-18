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

/**
 * @file aigmeta_coverage.hpp
 *
 * @brief Track coverage of AIG meta information
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef AIGMETA_COVERAGE_HPP
#define AIGMETA_COVERAGE_HPP

#include <map>

#include <classical/io/aigmeta.hpp>

namespace cirkit
{

struct aigmeta_coverage
{
  aigmeta_coverage( const aigmeta& meta ) : meta( meta ) {}

  void cover( const aigmeta_bundle& bundle, unsigned literal );
  double coverage() const;

  void print_report() const;

private:
  const aigmeta& meta;
  std::map<const aigmeta_bundle*, std::list<unsigned>> covered_bundles;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// End:
