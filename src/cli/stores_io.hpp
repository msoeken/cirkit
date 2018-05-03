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
 * @file stores_io.hpp
 *
 * @brief I/O commands for stores
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_CORE_STORES_IO_HPP
#define CLI_CORE_STORES_IO_HPP

#include <string>

#include <alice/alice.hpp>

#include <core/utils/bdd_utils.hpp>
#include <classical/aig.hpp>
#include <classical/mig/mig.hpp>
#include <classical/utils/expression_parser.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <classical/xmg/xmg.hpp>

namespace alice
{

using namespace cirkit;

ALICE_ADD_FILE_TYPE( aiger, "Aiger" )
ALICE_ADD_FILE_TYPE_READ_ONLY( bench, "Bench" )
ALICE_ADD_FILE_TYPE_WRITE_ONLY( edgelist, "Edge list" )
ALICE_ADD_FILE_TYPE( pla, "PLA" )
ALICE_ADD_FILE_TYPE_WRITE_ONLY( smt, "SMT-LIB2")
ALICE_ADD_FILE_TYPE( verilog, "Verilog" )
ALICE_ADD_FILE_TYPE_READ_ONLY( yig, "YIG" )

/******************************************************************************
 * bdd_function_t                                                             *
 ******************************************************************************/

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
inline bool can_write<mig_graph, io_verilog_tag_t>( command& cmd ) { return true; }

template<>
void write<mig_graph, io_verilog_tag_t>( const mig_graph& mig, const std::string& filename, const command& cmd );

template<>
inline bool can_read<mig_graph, io_verilog_tag_t>( command& cmd ) { return true; }

template<>
mig_graph read<mig_graph, io_verilog_tag_t>( const std::string& filename, const command& cmd );

/******************************************************************************
 * tt                                                                         *
 ******************************************************************************/

template<>
inline bool can_write<tt, io_pla_tag_t>( command& cmd ) { return true; }

template<>
void write<tt, io_pla_tag_t>( const tt& t, const std::string& filename, const command& cmd );

/******************************************************************************
 * xmg_graph                                                                  *
 ******************************************************************************/

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
