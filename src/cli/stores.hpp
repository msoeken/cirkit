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

#include <alice/command.hpp>

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

struct io_aiger_tag_t {};
struct io_bench_tag_t {};
struct io_edgelist_tag_t {};
struct io_pla_tag_t {};
struct io_smt_tag_t {};
struct io_verilog_tag_t {};
struct io_yig_tag_t {};

/******************************************************************************
 * bdd_function_t                                                             *
 ******************************************************************************/

template<>
struct store_info<bdd_function_t>
{
  static constexpr const char* key         = "bdds";
  static constexpr const char* option      = "bdd";
  static constexpr const char* mnemonic    = "b";
  static constexpr const char* name        = "BDD";
  static constexpr const char* name_plural = "BDDs";
};

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

template<>
inline bool can_read<bdd_function_t, io_pla_tag_t>( command& cmd ) { return true; }

template<>
bdd_function_t read<bdd_function_t, io_pla_tag_t>( const std::string& filename, const command& cmd );

template<>
inline bool can_write<bdd_function_t, io_pla_tag_t>( command& cmd ) { return true; }

template<>
void write<bdd_function_t, io_pla_tag_t>( const bdd_function_t& bdd, const std::string& filename, const command& cmd );

/******************************************************************************
 * aig_graph                                                                  *
 ******************************************************************************/

template<>
struct store_info<aig_graph>
{
  static constexpr const char* key         = "aigs";
  static constexpr const char* option      = "aig";
  static constexpr const char* mnemonic    = "a";
  static constexpr const char* name        = "AIG";
  static constexpr const char* name_plural = "AIGs";
};

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

template<>
bool can_read<aig_graph, io_aiger_tag_t>( command& cmd );

template<>
aig_graph read<aig_graph, io_aiger_tag_t>( const std::string& filename, const command& cmd );

template<>
inline bool can_read<aig_graph, io_bench_tag_t>( command& cmd ) { return true; }

template<>
aig_graph read<aig_graph, io_bench_tag_t>( const std::string& filename, const command& cmd );

template<>
inline bool can_write<aig_graph, io_aiger_tag_t>( command& cmd ) { return true; }

template<>
void write<aig_graph, io_aiger_tag_t>( const aig_graph& aig, const std::string& filename, const command& cmd );

template<>
inline bool can_read<aig_graph, io_verilog_tag_t>( command& cmd ) { return true; }

template<>
aig_graph read<aig_graph, io_verilog_tag_t>( const std::string& filename, const command& cmd );

template<>
inline bool can_write<aig_graph, io_verilog_tag_t>( command& cmd ) { return true; }

template<>
void write<aig_graph, io_verilog_tag_t>( const aig_graph& aig, const std::string& filename, const command& cmd );

template<>
inline bool can_write<aig_graph, io_edgelist_tag_t>( command& cmd ) { return true; }

template<>
void write<aig_graph, io_edgelist_tag_t>( const aig_graph& aig, const std::string& filename, const command& cmd );

/******************************************************************************
 * mig_graph                                                                  *
 ******************************************************************************/

template<>
struct store_info<mig_graph>
{
  static constexpr const char* key         = "migs";
  static constexpr const char* option      = "mig";
  static constexpr const char* mnemonic    = "m";
  static constexpr const char* name        = "MIG";
  static constexpr const char* name_plural = "MIGs";
};

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

template<>
inline bool can_write<mig_graph, io_verilog_tag_t>( command& cmd ) { return true; }

template<>
void write<mig_graph, io_verilog_tag_t>( const mig_graph& mig, const std::string& filename, const command& cmd );

template<>
inline bool can_read<mig_graph, io_verilog_tag_t>( command& cmd ) { return true; }

template<>
mig_graph read<mig_graph, io_verilog_tag_t>( const std::string& filename, const command& cmd );

/******************************************************************************
 * counterexample_t                                                           *
 ******************************************************************************/

template<>
struct store_info<counterexample_t>
{
  static constexpr const char* key         = "cex";
  static constexpr const char* option      = "counterexample";
  static constexpr const char* mnemonic    = "";
  static constexpr const char* name        = "counterexample";
  static constexpr const char* name_plural = "counterexamples";
};

template<>
std::string to_string<counterexample_t>( const counterexample_t& cex );

/******************************************************************************
 * simple_fanout_graph_t                                                      *
 ******************************************************************************/

template<>
struct store_info<simple_fanout_graph_t>
{
  static constexpr const char* key         = "nls";
  static constexpr const char* option      = "netlist";
  static constexpr const char* mnemonic    = "n";
  static constexpr const char* name        = "netlist";
  static constexpr const char* name_plural = "netlists";
};

template<>
std::string to_string<simple_fanout_graph_t>( const simple_fanout_graph_t& nl );

/******************************************************************************
 * std::vector<aig_node>                                                      *
 ******************************************************************************/

template<>
struct store_info<std::vector<aig_node>>
{
  static constexpr const char* key         = "gates";
  static constexpr const char* option      = "gate";
  static constexpr const char* mnemonic    = "";
  static constexpr const char* name        = "gate";
  static constexpr const char* name_plural = "gates";
};

template<>
std::string to_string<std::vector<aig_node>>( const std::vector<aig_node>& g );

template<>
void print<std::vector<aig_node>>( std::ostream& os, const std::vector<aig_node>& g );

/******************************************************************************
 * tt                                                                         *
 ******************************************************************************/

template<>
struct store_info<tt>
{
  static constexpr const char* key         = "tts";
  static constexpr const char* option      = "tt";
  static constexpr const char* mnemonic    = "t";
  static constexpr const char* name        = "truth table";
  static constexpr const char* name_plural = "truth tables";
};

template<>
std::string to_string<tt>( const tt& t );

template<>
void print<tt>( std::ostream& os, const tt& t );

template<>
inline bool can_write<tt, io_pla_tag_t>( command& cmd ) { return true; }

template<>
void write<tt, io_pla_tag_t>( const tt& t, const std::string& filename, const command& cmd );

/******************************************************************************
 * expression_t::ptr                                                          *
 ******************************************************************************/

template<>
struct store_info<expression_t::ptr>
{
  static constexpr const char* key         = "exprs";
  static constexpr const char* option      = "expr";
  static constexpr const char* mnemonic    = "e";
  static constexpr const char* name        = "expression";
  static constexpr const char* name_plural = "expressions";
};

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

template<>
struct store_info<xmg_graph>
{
  static constexpr const char* key         = "xmgs";
  static constexpr const char* option      = "xmg";
  static constexpr const char* mnemonic    = "x";
  static constexpr const char* name        = "XMG";
  static constexpr const char* name_plural = "XMGs";
};

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

template<>
inline bool can_write<xmg_graph, io_bench_tag_t>( command& cmd ) { return true; }

template<>
void write<xmg_graph, io_bench_tag_t>( const xmg_graph& xmg, const std::string& filename, const command& cmd );

template<>
bool can_read<xmg_graph, io_verilog_tag_t>( command& cmd );

template<>
xmg_graph read<xmg_graph, io_verilog_tag_t>( const std::string& filename, const command& cmd );

template<>
bool can_write<xmg_graph, io_verilog_tag_t>( command& cmd );

template<>
void write<xmg_graph, io_verilog_tag_t>( const xmg_graph& xmg, const std::string& filename, const command& cmd );

template<>
inline bool can_read<xmg_graph, io_yig_tag_t>( command& cmd ) { return true; }

template<>
xmg_graph read<xmg_graph, io_yig_tag_t>( const std::string& filename, const command& cmd );

template<>
bool can_write<xmg_graph, io_smt_tag_t>( command& cmd );

template<>
void write<xmg_graph, io_smt_tag_t>( const xmg_graph& xmg, const std::string& filename, const command& cmd );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
