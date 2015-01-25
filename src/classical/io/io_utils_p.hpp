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
 * @file io_utils_p.hpp
 *
 * @brief Private utility functions for write_bench and write_verilog
 *
 * @author Mathias Soeken
 * @since  2.2
 */

#ifndef AIG_IO_UTILS_P_HPP
#define AIG_IO_UTILS_P_HPP

#include <functional>
#include <string>

#include <classical/aig.hpp>

namespace cirkit
{

std::pair<aig_function, aig_function> get_operands( const aig_node& v, const aig_graph& aig );
std::string get_node_name( const aig_node& v, const aig_graph& aig, const std::string& prefix = "" );
std::string get_node_name_processed( const aig_node& v, const aig_graph& aig, const std::function<std::string(const std::string&)>& f, const std::string& prefix = "" );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
