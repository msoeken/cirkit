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
 * @file reversible_stores.hpp
 *
 * @brief Meta-data for stores
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef REVERSIBLE_STORES_HPP
#define REVERSIBLE_STORES_HPP

#include <alice/api.hpp>
#include <alice/store_api.hpp>

#include <cli/stores.hpp>
#include <classical/aig.hpp>
#include <classical/utils/expression_parser.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/truth_table.hpp>

namespace alice
{

using namespace cirkit;

/******************************************************************************
 * circuit                                                                    *
 ******************************************************************************/

ALICE_ADD_STORE( circuit, "circuit", "c", "circuit", "circuits" )

template<>
std::string to_string<circuit>( const circuit& circ );

template<>
void print<circuit>( std::ostream& os, const circuit& circ );

template<>
void print_statistics<circuit>( std::ostream& os, const circuit& circ );

template<>
nlohmann::json log_statistics<circuit>( const circuit& circ );

template<>
bool can_show<circuit>( std::string& extension, command& cmd );

template<>
void show<circuit>( std::ostream& out, const circuit& circ, const command& cmd );

template<>
inline bool can_convert<circuit, aig_graph>() { return true; }

template<>
aig_graph convert<circuit, aig_graph>( const circuit& circ );

template<>
inline bool has_html_repr<circuit>() { return true; }

template<>
std::string html_repr<circuit>( const circuit& circ );

/******************************************************************************
 * binary_truth_table                                                         *
 ******************************************************************************/

ALICE_ADD_STORE( binary_truth_table, "spec", "s", "specification", "specifications" )

template<>
std::string to_string<binary_truth_table>( const binary_truth_table& spec );

template<>
void print<binary_truth_table>( std::ostream& os, const binary_truth_table& spec );

template<>
inline bool can_convert<circuit, binary_truth_table>() { return true; }

template<>
binary_truth_table convert<circuit, binary_truth_table>( const circuit& circ );

template<>
inline bool can_convert<expression_t::ptr, binary_truth_table>() { return true; }

template<>
binary_truth_table convert<expression_t::ptr, binary_truth_table>( const expression_t::ptr& expr );

template<>
inline bool can_convert<tt, binary_truth_table>() { return true; }

template<>
binary_truth_table convert<tt, binary_truth_table>( const tt& func );

/******************************************************************************
 * rcbdd                                                                      *
 ******************************************************************************/

ALICE_ADD_STORE( rcbdd, "rcbdd", "r", "RCBDD", "RCBDDs" )

template<>
std::string to_string<rcbdd>( const rcbdd& bdd );

template<>
nlohmann::json log_statistics<rcbdd>( const rcbdd& bdd );

template<>
bool can_show<rcbdd>( std::string& extension, command& cmd );

template<>
void show<rcbdd>( std::ostream& out, const rcbdd& bdd, const command& cmd );

template<>
void print<rcbdd>( std::ostream& os, const rcbdd& bdd );

template<>
inline bool can_convert<circuit, rcbdd>() { return true; }

template<>
rcbdd convert<circuit, rcbdd>( const circuit& circ );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
