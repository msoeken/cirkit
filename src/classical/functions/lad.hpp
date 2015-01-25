/* CirKit: A circuit toolkit
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

/**
 * @file lad.hpp
 *
 * @brief LAD algorithm (Port to C++)
 *
 * Most of the code is from: http://liris.cnrs.fr/csolnon/LAD.html
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef LAD_HPP
#define LAD_HPP

#include <string>
#include <vector>

#include <core/properties.hpp>

#include <classical/aig.hpp>

namespace cirkit
{

bool directed_lad( std::vector<unsigned>& mapping, const std::string& target, const std::string& pattern,
                   properties::ptr settings = properties::ptr(), properties::ptr statistics = properties::ptr() );

bool directed_lad_from_aig( std::vector<unsigned>& mapping, const aig_graph& target, const aig_graph& pattern, unsigned selector,
                            properties::ptr settings = properties::ptr(), properties::ptr statistics = properties::ptr() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
