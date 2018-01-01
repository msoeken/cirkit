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
 * @file stores.hpp
 *
 * @brief Meta-data for stores
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_CORE_STORES_HPP
#define CLI_CORE_STORES_HPP

#include <string>
#include <vector>

#include <alice/alice.hpp>

#include <core/properties.hpp>
#include <core/utils/bdd_utils.hpp>
#include <classical/aig.hpp>
#include <classical/mig/mig.hpp>
#include <classical/netlist_graphs.hpp>
#include <classical/utils/aig_utils.hpp>
#include <classical/utils/counterexample.hpp>
#include <classical/utils/expression_parser.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <classical/xmg/xmg.hpp>

namespace alice
{

using namespace cirkit;

/******************************************************************************
 * bdd_function_t                                                             *
 ******************************************************************************/

ALICE_ADD_STORE( bdd_function_t, "bdd", "b", "BDD", "BDDs" )

template<>
std::string to_string<bdd_function_t>( const bdd_function_t& bdd );

template<>
void print<bdd_function_t>( std::ostream& os, const bdd_function_t& bdd );

// template<>
// struct show_store_entry<bdd_function_t>
// {
//   show_store_entry( command& cmd );

//   bool operator()( bdd_function_t& bdd, const std::string& dotname, const command& cmd );

//   command::log_opt_t log() const;
// };

template<>
void print_statistics<bdd_function_t>( std::ostream& os, const bdd_function_t& bdd );

template<>
nlohmann::json log_statistics<bdd_function_t>( const bdd_function_t& bdd );

/******************************************************************************
 * aig_graph                                                                  *
 ******************************************************************************/

ALICE_ADD_STORE( aig_graph, "aig", "a", "AIG", "AIGs" )

template<>
std::string to_string<aig_graph>( const aig_graph& aig );

// template<>
// struct show_store_entry<aig_graph>
// {
//   show_store_entry( command& cmd );

//   bool operator()( aig_graph& aig, const std::string& dotname, const command& cmd );

//   command::log_opt_t log() const;
// };

template<>
void print_statistics<aig_graph>( std::ostream& os, const aig_graph& aig );

template<>
nlohmann::json log_statistics<aig_graph>( const aig_graph& aig );

template<>
inline bool can_convert<tt, aig_graph>() { return true; }

template<>
aig_graph convert<tt, aig_graph>( const tt& t );

template<>
inline bool can_convert<aig_graph, bdd_function_t>() { return true; }

template<>
bdd_function_t convert<aig_graph, bdd_function_t>( const aig_graph& aig );

/******************************************************************************
 * mig_graph                                                                  *
 ******************************************************************************/

ALICE_ADD_STORE( mig_graph, "mig", "m", "MIG", "MIGs" )

template<>
std::string to_string<mig_graph>( const mig_graph& mig );

// template<>
// struct show_store_entry<mig_graph>
// {
//   show_store_entry( command& cmd );

//   bool operator()( mig_graph& mig, const std::string& dotname, const command& cmd );

//   command::log_opt_t log() const;

// private:
//   std::vector<std::string> expressions;
// };

template<>
void print_statistics<mig_graph>( std::ostream& os, const mig_graph& mig );

template<>
nlohmann::json log_statistics<mig_graph>( const mig_graph& mig );

template<>
inline bool can_convert<mig_graph, aig_graph>() { return true; }

template<>
aig_graph convert<mig_graph, aig_graph>( const mig_graph& mig );

template<>
inline bool can_convert<aig_graph, mig_graph>() { return true; }

template<>
mig_graph convert<aig_graph, mig_graph>( const aig_graph& mig );

template<>
inline bool can_convert<mig_graph, expression_t::ptr>() { return true; }

template<>
expression_t::ptr convert<mig_graph, expression_t::ptr>( const mig_graph& mig );

template<>
inline bool can_convert<expression_t::ptr, mig_graph>() { return true; }

template<>
mig_graph convert<expression_t::ptr, mig_graph>( const expression_t::ptr& expr );

/******************************************************************************
 * counterexample_t                                                           *
 ******************************************************************************/

ALICE_ADD_STORE( counterexample_t, "counterexample", "", "counterexample", "counterexamples" )

template<>
std::string to_string<counterexample_t>( const counterexample_t& cex );

/******************************************************************************
 * simple_fanout_graph_t                                                      *
 ******************************************************************************/

ALICE_ADD_STORE( simple_fanout_graph_t, "netlist", "", "netlist", "netlists" )

template<>
std::string to_string<simple_fanout_graph_t>( const simple_fanout_graph_t& nl );

/******************************************************************************
 * std::vector<aig_node>                                                      *
 ******************************************************************************/

ALICE_ADD_STORE( std::vector<aig_node>, "gate", "", "gate", "gates" )

template<>
std::string to_string<std::vector<aig_node>>( const std::vector<aig_node>& g );

template<>
void print<std::vector<aig_node>>( std::ostream& os, const std::vector<aig_node>& g );

/******************************************************************************
 * tt                                                                         *
 ******************************************************************************/

ALICE_ADD_STORE( tt, "tt", "t", "truth table", "truth tables" )

template<>
std::string to_string<tt>( const tt& t );

template<>
void print<tt>( std::ostream& os, const tt& t );

/******************************************************************************
 * expression_t::ptr                                                          *
 ******************************************************************************/

ALICE_ADD_STORE( expression_t::ptr, "expr", "e", "expression", "expressions" )

template<>
std::string to_string<expression_t::ptr>( const expression_t::ptr& expr );

template<>
void print_statistics<expression_t::ptr>( std::ostream& os, const expression_t::ptr& expr );

template<>
nlohmann::json log_statistics<expression_t::ptr>( const expression_t::ptr& expr );

template<>
void print<expression_t::ptr>( std::ostream& os, const expression_t::ptr& expr );

template<>
inline bool can_convert<expression_t::ptr, tt>() { return true; }

template<>
tt convert<expression_t::ptr, tt>( const expression_t::ptr& expr );

template<>
inline bool can_convert<expression_t::ptr, bdd_function_t>() { return true; }

template<>
bdd_function_t convert<expression_t::ptr, bdd_function_t>( const expression_t::ptr& expr );

/******************************************************************************
 * xmg_graph                                                                  *
 ******************************************************************************/

ALICE_ADD_STORE( xmg_graph, "xmg", "x", "XMG", "XMGs" )

template<>
std::string to_string<xmg_graph>( const xmg_graph& xmg );

template<>
void print_statistics<xmg_graph>( std::ostream& os, const xmg_graph& xmg );

template<>
nlohmann::json log_statistics<xmg_graph>( const xmg_graph& xmg );

// template<>
// struct show_store_entry<xmg_graph>
// {
//   show_store_entry( command& cmd );

//   bool operator()( xmg_graph& mig, const std::string& dotname, const command& cmd );

//   command::log_opt_t log() const;
// };

template<>
inline bool can_convert<xmg_graph, expression_t::ptr>() { return true; }

template<>
expression_t::ptr convert<xmg_graph, expression_t::ptr>( const xmg_graph& xmg );

template<>
inline bool can_convert<expression_t::ptr, xmg_graph>() { return true; }

template<>
xmg_graph convert<expression_t::ptr, xmg_graph>( const expression_t::ptr& expr );

template<>
inline bool can_convert<aig_graph, xmg_graph>() { return true; }

template<>
xmg_graph convert<aig_graph, xmg_graph>( const aig_graph& aig );

template<>
inline bool can_convert<xmg_graph, aig_graph>() { return true; }

template<>
aig_graph convert<xmg_graph, aig_graph>( const xmg_graph& aig );

template<>
inline bool can_convert<mig_graph, xmg_graph>() { return true; }

template<>
xmg_graph convert<mig_graph, xmg_graph>( const mig_graph& mig );

template<>
inline bool can_convert<xmg_graph, mig_graph>() { return true; }

template<>
mig_graph convert<xmg_graph, mig_graph>( const xmg_graph& mig );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
