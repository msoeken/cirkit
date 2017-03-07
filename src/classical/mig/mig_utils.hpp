/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2017  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
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
