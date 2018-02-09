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

#include "reversible_stores_io.hpp"

#include <cstdlib>
#include <vector>

#include <core/utils/string_utils.hpp>
#include <core/utils/system_utils.hpp>
#include <core/utils/temporary_filename.hpp>
#include <reversible/functions/circuit_from_string.hpp>
#include <reversible/functions/permutation_to_truth_table.hpp>
#include <reversible/io/create_image.hpp>
#include <reversible/io/read_qc.hpp>
#include <reversible/io/read_realization.hpp>
#include <reversible/io/read_specification.hpp>
#include <reversible/io/write_liquid.hpp>
#include <reversible/io/write_numpy.hpp>
#include <reversible/io/write_pla.hpp>
#include <reversible/io/write_projectq.hpp>
#include <reversible/io/write_pyquil.hpp>
#include <reversible/io/write_qc.hpp>
#include <reversible/io/write_qcode.hpp>
#include <reversible/io/write_qiskit.hpp>
#include <reversible/io/write_qpic.hpp>
#include <reversible/io/write_qsharp.hpp>
#include <reversible/io/write_quipper.hpp>
#include <reversible/io/write_realization.hpp>
#include <reversible/io/write_specification.hpp>

namespace alice
{

using namespace cirkit;

/******************************************************************************
 * circuit                                                                    *
 ******************************************************************************/

template<>
bool can_write<circuit, io_qpic_tag_t>( command& cmd )
{
  cmd.add_flag( "--print_index", "prints index below each gate" );
  return true;
}

template<>
void write<circuit, io_qpic_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
{
  auto settings = std::make_shared<properties>();
  settings->set( "print_index", cmd.is_set( "print_index" ) );
  write_qpic( circ, filename, settings );
}

template<>
bool can_write<circuit, io_quipper_tag_t>( command& cmd )
{
  cmd.add_flag( "--ascii", "write ASCII instead of Haskell program" );
  return true;
}

template<>
void write<circuit, io_quipper_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
{
  if ( cmd.is_set( "ascii" ) )
  {
    write_quipper_ascii( circ, filename );
  }
  else
  {
    write_quipper( circ, filename );
  }
}

template<>
bool can_write<circuit, io_liquid_tag_t>( command& cmd )
{
  cmd.add_flag( "--dump", "wite dump statement into script" );
  return true;
}

template<>
void write<circuit, io_liquid_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
{
  auto settings = std::make_shared<properties>();
  settings->set( "dump_statement", cmd.is_set( "dump" ) );

  write_liquid( circ, filename, settings );
}

template<>
bool can_read<circuit, io_real_tag_t>( command& cmd )
{
  cmd.add_option<std::string>( "--string,-s", "read from string (e.g. t3 a b c, t2 a b, t1 a, f3 a b c)" );
  return true;
}

template<>
circuit read<circuit, io_real_tag_t>( const std::string& filename, const command& cmd )
{
  if ( cmd.is_set( "string" ) )
  {
    return circuit_from_string( cmd.option_value<std::string>( "--string" ) );
  }
  else
  {
    circuit circ;
    read_realization( circ, filename );
    return circ;
  }
}

template<>
bool can_write<circuit, io_real_tag_t>( command& cmd )
{
  cmd.add_flag( "--string,-s", "write to string (which can be read with read_real -s)" );
  return true;
}

template<>
void write<circuit, io_real_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
{
  if ( cmd.is_set( "string" ) )
  {
    std::cout << circuit_to_string( circ ) << std::endl;
  }
  else
  {
    write_realization( circ, filename );
  }
}

template<>
bool can_write<circuit, io_tikz_tag_t>( command& cmd )
{
  cmd.add_flag( "--standalone", "surround Tikz code with LaTeX template to compile" )->group( "Circuits" );
  cmd.add_flag( "--hideio", "don't print I/O" )->group( "Circuits" );
  return true;
}

template<>
void write<circuit, io_tikz_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
{
  create_tikz_settings ct_settings;

  ct_settings.draw_io = !cmd.is_set( "hideio" );

  std::ofstream os( filename.c_str() );

  if ( cmd.is_set( "standalone" ) )
  {
    os << "\\documentclass{standalone}" << std::endl
       << "\\usepackage{tikz}" << std::endl << std::endl
       << "\\begin{document}" << std::endl;
  }
  create_image( os, circ, ct_settings );

  if ( cmd.is_set( "standalone" ) )
  {
    os << "\\end{document}" << std::endl;
  }
}

template<>
circuit read<circuit, io_qc_tag_t>( const std::string& filename, const command& cmd )
{
  return read_qc( filename );
}

template<>
bool can_write<circuit, io_qc_tag_t>( command& cmd )
{
  cmd.add_flag( "--iqc", "write IQC compliant files" );
  return true;
}

template<>
void write<circuit, io_qc_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
{
  write_qc( circ, filename, cmd.is_set( "iqc" ) );
}

template<>
void write<circuit, io_qcode_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
{
  write_qcode( circ, filename );
}

template<>
void write<circuit, io_qiskit_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
{
  write_qiskit( circ, filename );
}

template<>
void write<circuit, io_pyquil_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
{
  write_pyquil( circ, filename );
}

template<>
void write<circuit, io_numpy_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
{
  write_numpy( circ, filename );
}

template<>
void write<circuit, io_projectq_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
{
  write_projectq( circ, filename );
}

template<>
bool can_write<circuit, io_qsharp_tag_t>( command& cmd )
{
  cmd.add_option<std::string>( "--namespace", "name for the namespace" );
  cmd.add_option<std::string>( "--operation", "name for the operation" );
  return true;
}

template<>
void write<circuit, io_qsharp_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
{
  auto settings = std::make_shared<properties>();
  settings->set( "namespace_name", cmd.option_value<std::string>( "--namespace", "RevKit.Compilation" ) );
  settings->set( "operation_name", cmd.option_value<std::string>( "--operation", "Oracle" ) );
  write_qsharp( circ, filename, settings );
}

/******************************************************************************
 * binary_truth_table                                                         *
 ******************************************************************************/

template<>
bool can_read<binary_truth_table, io_spec_tag_t>( command& cmd )
{
  cmd.add_option<std::string>( "--permutation,-p", "create spec from permutation (starts with 0, space separated)" )->group( "Specification" );
  return true;
}

template<>
binary_truth_table read<binary_truth_table, io_spec_tag_t>( const std::string& filename, const command& cmd )
{
  if ( cmd.is_set( "permutation" ) )
  {
    std::vector<unsigned> perm;
    parse_string_list( perm, cmd.option_value<std::string>( "--permutation" ) );

    return permutation_to_truth_table( perm );
  }
  else
  {
    binary_truth_table spec;
    read_specification( spec, filename );
    return spec;
  }
}

template<>
void write<binary_truth_table, io_spec_tag_t>( const binary_truth_table& spec, const std::string& filename, const command& cmd )
{
  write_specification( spec, filename );
}

template<>
void write<binary_truth_table, io_pla_tag_t>( const binary_truth_table& spec, const std::string& filename, const command& cmd )
{
  write_pla( spec, filename );
}

/******************************************************************************
 * rcbdd                                                                      *
 ******************************************************************************/

template<>
bool can_write<rcbdd, io_pla_tag_t>( command& cmd )
{
  cmd.add_flag( "--full", "also write constants and garbage (only for RCBDD)" )->group( "RCBDDs" );
  return true;
}

template<>
void write<rcbdd, io_pla_tag_t>( const rcbdd& bdd, const std::string& filename, const command& cmd )
{
  bdd.write_pla( filename, cmd.is_set( "full" ) );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
