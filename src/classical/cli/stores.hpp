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
 * @file stores.hpp
 *
 * @brief Meta-data for stores
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_CLASSICAL_STORES_HPP
#define CLI_CLASSICAL_STORES_HPP

#include <string>
#include <vector>

#include <core/properties.hpp>
#include <core/cli/store.hpp>

#include <classical/aig.hpp>
#include <classical/netlist_graphs.hpp>
#include <classical/utils/aig_utils.hpp>
#include <classical/utils/counterexample.hpp>
#include <classical/utils/truth_table_utils.hpp>

namespace cirkit
{

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
std::string store_entry_to_string<aig_graph>( const aig_graph& aig );

template<>
struct show_store_entry<aig_graph>
{
  show_store_entry( program_options& opts );

  bool operator()( aig_graph& aig, const std::string& dotname, const program_options& opts, const properties::ptr& settings );

  command_log_opt_t log() const;
};

template<>
void print_store_entry_statistics<aig_graph>( std::ostream& os, const aig_graph& aig );

template<>
command_log_opt_t log_store_entry_statistics<aig_graph>( const aig_graph& aig );

template<>
inline bool store_can_convert<tt, aig_graph>() { return true; }

template<>
aig_graph store_convert<tt, aig_graph>( const tt& t );

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
std::string store_entry_to_string<counterexample_t>( const counterexample_t& cex );

/******************************************************************************
 * simple_fanout_graph_t                                                      *
 ******************************************************************************/

template<>
struct store_info<simple_fanout_graph_t>
{
  static constexpr const char* key         = "nls";
  static constexpr const char* option      = "netlists";
  static constexpr const char* mnemonic    = "n";
  static constexpr const char* name        = "netlist";
  static constexpr const char* name_plural = "netlists";
};

template<>
std::string store_entry_to_string<simple_fanout_graph_t>( const simple_fanout_graph_t& nl );

/******************************************************************************
 * std::vector<aig_node>                                                      *
 ******************************************************************************/

template<>
struct store_info<std::vector<aig_node>>
{
  static constexpr const char* key         = "gates";
  static constexpr const char* option      = "gates";
  static constexpr const char* mnemonic    = "g";
  static constexpr const char* name        = "gate";
  static constexpr const char* name_plural = "gates";
};

template<>
std::string store_entry_to_string<std::vector<aig_node>>( const std::vector<aig_node>& g );

template<>
void print_store_entry<std::vector<aig_node>>( std::ostream& os, const std::vector<aig_node>& g );

/******************************************************************************
 * tt                                                                         *
 ******************************************************************************/

template<>
struct store_info<tt>
{
  static constexpr const char* key         = "tts";
  static constexpr const char* option      = "tts";
  static constexpr const char* mnemonic    = "t";
  static constexpr const char* name        = "truth table";
  static constexpr const char* name_plural = "truth tables";
};

template<>
std::string store_entry_to_string<tt>( const tt& t );

template<>
void print_store_entry<tt>( std::ostream& os, const tt& t );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
