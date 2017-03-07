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
