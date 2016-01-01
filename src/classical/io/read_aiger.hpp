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
 * @file read_aiger.hpp
 *
 * @brief Read AIGs in ASCII AIGER format
 *
 * @author Heinz Riener
 * @since  2.0
 */

#ifndef READ_AIGER_HPP
#define READ_AIGER_HPP

#include <classical/aig.hpp>
#include <iostream>
#include <string>

namespace cirkit
{

unsigned aiger_lit2var( const unsigned lit );

void read_aiger( aig_graph& aig, std::istream& in );
void read_aiger( aig_graph& aig, const std::string& filename );
void read_aiger( aig_graph& aig, std::string& comment, std::istream& in );
void read_aiger( aig_graph& aig, std::string& comment, const std::string& filename );

void read_aiger_binary( aig_graph& aig, std::istream& in, bool noopt = false );
void read_aiger_binary( aig_graph& aig, const std::string& filename, bool noopt = false );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
