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
 * @file aig_utils.hpp
 *
 * @brief AIG utils
 *
 * @author Heinz Riener
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef AIG_UTILS_HPP
#define AIG_UTILS_HPP

#include <classical/aig.hpp>

#include <string>
#include <iostream>
#include <vector>

namespace cirkit
{

inline const aig_graph_info& aig_info( const aig_graph& aig ) { return boost::get_property( aig, boost::graph_name ); }
inline       aig_graph_info& aig_info(       aig_graph& aig ) { return boost::get_property( aig, boost::graph_name ); }

bool split_name( std::string& na, unsigned& idx, const std::string& name );
void aig_mirror_word_names( aig_graph& aig );
aig_node aig_node_by_name( const aig_graph& aig, const std::string& name );
aig_node aig_node_by_name( const aig_graph_info& info, const std::string& name );
unsigned aig_input_index( const aig_graph& aig, const aig_node& input );
unsigned aig_input_index( const aig_graph_info& info, const aig_node& input );
unsigned aig_output_index( const aig_graph& aig, const std::string& name );
unsigned aig_output_index( const aig_graph_info& info, const std::string& name );
void aig_print_stats( const aig_graph& aig, std::ostream& os = std::cout );
std::vector<aig_function> get_children( const aig_graph& aig, const aig_node& node );
aig_function make_function( const aig_function& f, bool complemented );

}

#endif /* AIG_UTILS_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
