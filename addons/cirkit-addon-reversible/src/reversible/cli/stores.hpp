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

#ifndef STORES_HPP
#define STORES_HPP

#include <alice/command.hpp>

#include <core/cli/stores.hpp>
#include <classical/aig.hpp>
#include <classical/utils/expression_parser.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/truth_table.hpp>

namespace alice
{

using namespace cirkit;

struct io_real_tag_t {};
struct io_spec_tag_t {};
struct io_quipper_tag_t {};
struct io_liquid_tag_t {};
struct io_tikz_tag_t {};
struct io_numpy_tag_t {};
struct io_projectq_tag_t {};
struct io_qpic_tag_t {};
struct io_qc_tag_t {};
struct io_qcode_tag_t {};

/******************************************************************************
 * circuit                                                                    *
 ******************************************************************************/

template<>
struct store_info<circuit>
{
  static constexpr const char* key         = "circuits";
  static constexpr const char* option      = "circuit";
  static constexpr const char* mnemonic    = "c";
  static constexpr const char* name        = "circuit";
  static constexpr const char* name_plural = "circuits";
};

template<>
std::string store_entry_to_string<circuit>( const circuit& circ );

template<>
void print_store_entry<circuit>( std::ostream& os, const circuit& circ );

template<>
void print_store_entry_statistics<circuit>( std::ostream& os, const circuit& circ );

template<>
command::log_opt_t log_store_entry_statistics<circuit>( const circuit& circ );

template<>
struct show_store_entry<circuit>
{
  show_store_entry( const command& cmd );

  bool operator()( circuit& circ, const std::string& dotname, const command& cmd );

  command::log_opt_t log() const;
};

template<>
inline bool store_can_convert<circuit, aig_graph>() { return true; }

template<>
aig_graph store_convert<circuit, aig_graph>( const circuit& circ );

template<>
bool store_can_write_io_type<circuit, io_qpic_tag_t>( command& cmd );

template<>
void store_write_io_type<circuit, io_qpic_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

template<>
bool store_can_write_io_type<circuit, io_quipper_tag_t>( command& cmd );

template<>
void store_write_io_type<circuit, io_quipper_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

template<>
bool store_can_write_io_type<circuit, io_liquid_tag_t>( command& cmd );

template<>
void store_write_io_type<circuit, io_liquid_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

template<>
bool store_can_read_io_type<circuit, io_real_tag_t>( command& cmd );

template<>
circuit store_read_io_type<circuit, io_real_tag_t>( const std::string& filename, const command& cmd );

template<>
bool store_can_write_io_type<circuit, io_real_tag_t>( command& cmd );

template<>
void store_write_io_type<circuit, io_real_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

template<>
bool store_can_write_io_type<circuit, io_tikz_tag_t>( command& cmd );

template<>
void store_write_io_type<circuit, io_tikz_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

template<>
inline bool store_can_read_io_type<circuit, io_qc_tag_t>( command& cmd ) { return true; }

template<>
circuit store_read_io_type<circuit, io_qc_tag_t>( const std::string& filename, const command& cmd );

template<>
bool store_can_write_io_type<circuit, io_qc_tag_t>( command& cmd );

template<>
void store_write_io_type<circuit, io_qc_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

template<>
inline bool store_can_write_io_type<circuit, io_qcode_tag_t>( command& cmd ) { return true; }

template<>
void store_write_io_type<circuit, io_qcode_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

template<>
inline bool store_can_write_io_type<circuit, io_numpy_tag_t>( command& cmd ) { return true; }

template<>
void store_write_io_type<circuit, io_numpy_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

template<>
inline bool store_can_write_io_type<circuit, io_projectq_tag_t>( command& cmd ) { return true; }

template<>
void store_write_io_type<circuit, io_projectq_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

template<>
inline bool store_has_repr_html<circuit>() { return true; }

template<>
std::string store_repr_html<circuit>( const circuit& circ );

/******************************************************************************
 * binary_truth_table                                                         *
 ******************************************************************************/

template<>
struct store_info<binary_truth_table>
{
  static constexpr const char* key         = "spec";
  static constexpr const char* option      = "spec";
  static constexpr const char* mnemonic    = "s";
  static constexpr const char* name        = "specification";
  static constexpr const char* name_plural = "specifications";
};

template<>
std::string store_entry_to_string<binary_truth_table>( const binary_truth_table& spec );

template<>
void print_store_entry<binary_truth_table>( std::ostream& os, const binary_truth_table& spec );

template<>
inline bool store_can_convert<circuit, binary_truth_table>() { return true; }

template<>
binary_truth_table store_convert<circuit, binary_truth_table>( const circuit& circ );

template<>
inline bool store_can_convert<expression_t::ptr, binary_truth_table>() { return true; }

template<>
binary_truth_table store_convert<expression_t::ptr, binary_truth_table>( const expression_t::ptr& expr );

template<>
inline bool store_can_convert<tt, binary_truth_table>() { return true; }

template<>
binary_truth_table store_convert<tt, binary_truth_table>( const tt& func );

template<>
bool store_can_read_io_type<binary_truth_table, io_spec_tag_t>( command& cmd );

template<>
binary_truth_table store_read_io_type<binary_truth_table, io_spec_tag_t>( const std::string& filename, const command& cmd );

template<>
inline bool store_can_write_io_type<binary_truth_table, io_spec_tag_t>( command& cmd ) { return true; }

template<>
void store_write_io_type<binary_truth_table, io_spec_tag_t>( const binary_truth_table& spec, const std::string& filename, const command& cmd );

template<>
inline bool store_can_write_io_type<binary_truth_table, io_pla_tag_t>( command& cmd ) { return true; }

template<>
void store_write_io_type<binary_truth_table, io_pla_tag_t>( const binary_truth_table& spec, const std::string& filename, const command& cmd );

/******************************************************************************
 * rcbdd                                                                      *
 ******************************************************************************/

template<>
struct store_info<rcbdd>
{
  static constexpr const char* key         = "rcbdds";
  static constexpr const char* option      = "rcbdd";
  static constexpr const char* mnemonic    = "r";
  static constexpr const char* name        = "RCBDD";
  static constexpr const char* name_plural = "RCBDDs";
};

template<>
std::string store_entry_to_string<rcbdd>( const rcbdd& bdd );

template<>
command::log_opt_t log_store_entry_statistics<rcbdd>( const rcbdd& bdd );

template<>
struct show_store_entry<rcbdd>
{
  show_store_entry( const command& cmd );

  bool operator()( rcbdd& bdd, const std::string& dotname, const command& cmd );

  command::log_opt_t log() const;
};

template<>
void print_store_entry<rcbdd>( std::ostream& os, const rcbdd& bdd );

template<>
inline bool store_can_convert<circuit, rcbdd>() { return true; }

template<>
rcbdd store_convert<circuit, rcbdd>( const circuit& circ );

template<>
bool store_can_write_io_type<rcbdd, io_pla_tag_t>( command& cmd );

template<>
void store_write_io_type<rcbdd, io_pla_tag_t>( const rcbdd& bdd, const std::string& filename, const command& cmd );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
