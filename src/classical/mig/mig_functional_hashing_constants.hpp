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
 * @file mig_functional_hashing_constants.hpp
 *
 * @brief Minimal circuits for functional hashing
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef MIG_FUNCTIONAL_HASHING_CONSTANTS_HPP
#define MIG_FUNCTIONAL_HASHING_CONSTANTS_HPP

#include <map>
#include <string>
#include <unordered_map>

namespace cirkit
{

struct mig_functional_hashing_constants
{
  static const std::unordered_map<unsigned, std::tuple<unsigned, unsigned, std::map<char, unsigned>, std::string>> min_mig_sizes;
  static const std::unordered_map<unsigned, std::tuple<unsigned, unsigned, std::map<char, unsigned>, std::string>> min_depth_mig_sizes;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
