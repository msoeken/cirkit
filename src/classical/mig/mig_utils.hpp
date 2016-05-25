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
 * @file mig_utils.hpp
 *
 * @brief MIG utils
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef MIG_UTILS_HPP
#define MIG_UTILS_HPP

#include <iostream>
#include <map>

#include <classical/mig/mig.hpp>

namespace cirkit
{

inline const mig_graph_info& mig_info( const mig_graph& mig ) { return boost::get_property( mig, boost::graph_name ); }
inline       mig_graph_info& mig_info(       mig_graph& mig ) { return boost::get_property( mig, boost::graph_name ); }

inline mig_function make_function( const mig_function& f, bool complemented )
{
  if ( complemented )
  {
    return !f;
  }
  else
  {
    return std::move( f );
  }
}

void mig_print_stats( const mig_graph& mig, std::ostream& os = std::cout );
std::vector<mig_function> get_children( const mig_graph& mig, const mig_node& node );

unsigned number_of_complemented_edges( const mig_graph& mig );
unsigned number_of_inverters( const mig_graph& mig );

std::map<mig_node, unsigned> compute_levels( const mig_graph& mig, unsigned& max_level, bool push_to_outputs = false );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
