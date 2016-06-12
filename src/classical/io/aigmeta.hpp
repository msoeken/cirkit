/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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
 * @file aigmeta.hpp
 *
 * @brief AIG metadata files
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef AIGMETA_HPP
#define AIGMETA_HPP

#include <functional>
#include <iostream>
#include <list>
#include <string>
#include <vector>

namespace cirkit
{

struct aigmeta_port
{
  unsigned id;
  std::string name;
};

struct aigmeta_box
{
  unsigned id;
  std::string oper_type;
  std::vector<aigmeta_port> ports;
};

struct aigmeta_bundle
{
  unsigned id;
  std::string name;
  std::list<unsigned> literals;
};

struct aigmeta
{
  std::string dut;
  std::vector<aigmeta_box> boxes;
  std::vector<aigmeta_bundle> bundles;
};

void read_aigmeta( aigmeta& meta, const std::string& filename );
void write_aigmeta( const aigmeta& meta, const std::string& filename );

/**
 * @brief Iterates through bundles that contain literal
 *
 * @param meta    AIG meta-data
 * @param literal Literal to look for
 * @param exact   If false, then also literal + 1 is a valid match
 * @param f       Functor (first param is found bundle,
 *                second param is the literal that was found,
 *                and third param is the position of that literal in the bundle)
 */
void foreach_bundle_with_literal( const aigmeta& meta, unsigned literal, bool exact, const std::function<void(const aigmeta_bundle&, unsigned, unsigned)>& f );

std::ostream& operator<<( std::ostream& os, const aigmeta& meta );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
