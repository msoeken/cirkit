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
 * @file write_aiger.hpp
 *
 * @brief Write AIG to aiger format
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef WRITE_AIGER_HPP
#define WRITE_AIGER_HPP

#include <iostream>
#include <string>

#include <classical/aig.hpp>

namespace cirkit
{

void write_aiger( const aig_graph& aig, std::ostream& os );
void write_aiger( const aig_graph& aig, const std::string& filename );

}

#endif

// Local Variables:
// c-basic-offset: 2
// End:
