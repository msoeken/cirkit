/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
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
 * @file read_aigmeta.hpp
 *
 * @brief Read AIG metadata files
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef READ_AIGMETA_HPP
#define READ_AIGMETA_HPP

#include <iostream>
#include <list>
#include <string>
#include <vector>

namespace revkit
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
std::ostream& operator<<( std::ostream& os, const aigmeta& meta );

}

#endif

// Local Variables:
// c-basic-offset: 2
// End:
