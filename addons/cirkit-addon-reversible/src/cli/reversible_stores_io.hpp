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
 * @file reversible_stores_io.hpp
 *
 * @brief Meta-data for stores I/O
 *
 * @author Mathias Soeken
 * @since  2.4
 */

#ifndef REVERSIBLE_STORES_IO_HPP
#define REVERSIBLE_STORES_IO_HPP

#include <iostream>
#include <string>

#include <cli/stores_io.hpp>
#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/truth_table.hpp>

#include <alice/alice.hpp>

namespace alice
{

using namespace cirkit;

ALICE_ADD_FILE_TYPE( qc, "QC" )
ALICE_ADD_FILE_TYPE( real, "realization" )
ALICE_ADD_FILE_TYPE( spec, "specification" )
ALICE_ADD_FILE_TYPE_WRITE_ONLY( liquid, "LiQUi|>" )
ALICE_ADD_FILE_TYPE_WRITE_ONLY( numpy, "NumPy" )
ALICE_ADD_FILE_TYPE_WRITE_ONLY( projectq, "ProjectQ" )
ALICE_ADD_FILE_TYPE_WRITE_ONLY( qcode, "QCode" )
ALICE_ADD_FILE_TYPE_WRITE_ONLY( qpic, "qpic" )
ALICE_ADD_FILE_TYPE_WRITE_ONLY( qsharp, "Q#" )
ALICE_ADD_FILE_TYPE_WRITE_ONLY( quipper, "Quipper" )
ALICE_ADD_FILE_TYPE_WRITE_ONLY( tikz, "TikZ" )

/******************************************************************************
 * circuit                                                                    *
 ******************************************************************************/

template<>
bool can_write<circuit, io_qpic_tag_t>( command& cmd );

template<>
void write<circuit, io_qpic_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

template<>
bool can_write<circuit, io_quipper_tag_t>( command& cmd );

template<>
void write<circuit, io_quipper_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

template<>
bool can_write<circuit, io_liquid_tag_t>( command& cmd );

template<>
void write<circuit, io_liquid_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

template<>
bool can_read<circuit, io_real_tag_t>( command& cmd );

template<>
circuit read<circuit, io_real_tag_t>( const std::string& filename, const command& cmd );

template<>
bool can_write<circuit, io_real_tag_t>( command& cmd );

template<>
void write<circuit, io_real_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

template<>
bool can_write<circuit, io_tikz_tag_t>( command& cmd );

template<>
void write<circuit, io_tikz_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

template<>
inline bool can_read<circuit, io_qc_tag_t>( command& cmd ) { return true; }

template<>
circuit read<circuit, io_qc_tag_t>( const std::string& filename, const command& cmd );

template<>
bool can_write<circuit, io_qc_tag_t>( command& cmd );

template<>
void write<circuit, io_qc_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

template<>
inline bool can_write<circuit, io_qcode_tag_t>( command& cmd ) { return true; }

template<>
void write<circuit, io_qcode_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

template<>
inline bool can_write<circuit, io_numpy_tag_t>( command& cmd ) { return true; }

template<>
void write<circuit, io_numpy_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

template<>
inline bool can_write<circuit, io_projectq_tag_t>( command& cmd ) { return true; }

template<>
void write<circuit, io_projectq_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

template<>
bool can_write<circuit, io_qsharp_tag_t>( command& cmd );

template<>
void write<circuit, io_qsharp_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

/******************************************************************************
 * binary_truth_table                                                         *
 ******************************************************************************/

template<>
bool can_read<binary_truth_table, io_spec_tag_t>( command& cmd );

template<>
binary_truth_table read<binary_truth_table, io_spec_tag_t>( const std::string& filename, const command& cmd );

template<>
inline bool can_write<binary_truth_table, io_spec_tag_t>( command& cmd ) { return true; }

template<>
void write<binary_truth_table, io_spec_tag_t>( const binary_truth_table& spec, const std::string& filename, const command& cmd );

template<>
inline bool can_write<binary_truth_table, io_pla_tag_t>( command& cmd ) { return true; }

template<>
void write<binary_truth_table, io_pla_tag_t>( const binary_truth_table& spec, const std::string& filename, const command& cmd );

/******************************************************************************
 * rcbdd                                                                      *
 ******************************************************************************/

template<>
bool can_write<rcbdd, io_pla_tag_t>( command& cmd );

template<>
void write<rcbdd, io_pla_tag_t>( const rcbdd& bdd, const std::string& filename, const command& cmd );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
