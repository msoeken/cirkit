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

#include "reversible_stores.hpp"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include <core/utils/string_utils.hpp>
#include <core/utils/system_utils.hpp>
#include <core/utils/temporary_filename.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <reversible/functions/circuit_from_string.hpp>
#include <reversible/functions/circuit_to_aig.hpp>
#include <reversible/functions/circuit_to_truth_table.hpp>
#include <reversible/functions/permutation_to_truth_table.hpp>
#include <reversible/functions/truth_table_from_bitset.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/io/print_statistics.hpp>
#include <reversible/io/write_qpic.hpp>
#include <reversible/simulation/simple_simulation.hpp>
#include <reversible/utils/circuit_utils.hpp>
#include <reversible/utils/costs.hpp>

#include <fmt/format.h>
#include <json.hpp>

namespace alice
{

using namespace cirkit;

/******************************************************************************
 * circuit                                                                    *
 ******************************************************************************/

template<>
std::string to_string<circuit>( const circuit& circ )
{
  return fmt::format( "{} lines, {} gates", circ.lines(), circ.num_gates() );
}

template<>
void print<circuit>( std::ostream& os, const circuit& circ )
{
  os << circ << std::endl;
}

template<>
void print_statistics<circuit>( std::ostream& os, const circuit& circ )
{
  print_statistics_settings settings;
  settings.main_template = "%1$sLines:        %2$d\nGates:        %3$d\nT-count:      %6$d\nLogic qubits: %8$d\n";
  print_statistics( os, circ, -1.0, settings );
}

template<>
nlohmann::json log_statistics<circuit>( const circuit& circ )
{
  return nlohmann::json({
      {"gates", circ.num_gates()},
      {"lines", circ.lines()},
      {"tdepth", costs( circ, costs_by_gate_func( t_depth_costs() ) )},
      {"tcount", costs( circ, costs_by_gate_func( t_costs() ) )},
      {"ncv",    costs( circ, costs_by_gate_func( ncv_quantum_costs() ) )},
      {"qubits", number_of_qubits( circ )},
      {"depth", costs( circ, costs_by_circuit_func( depth_costs() ) )}
    });
}

template<>
bool can_show<circuit>( std::string& extension, command& cmd )
{
  extension = "svg";
  return false; /* for now */
}

template<>
void show<circuit>( std::ostream& out, const circuit& circ, const command& cmd )
{
  /* wait for good SVG output engine */
}

template<>
aig_graph convert<circuit, aig_graph>( const circuit& circ )
{
  return circuit_to_aig( circ );
}

template<>
binary_truth_table convert<expression_t::ptr, binary_truth_table>( const expression_t::ptr& expr )
{
  return truth_table_from_bitset_bennett( tt_from_expression( expr ) );
}

template<>
binary_truth_table convert<tt, binary_truth_table>( const tt& func )
{
  return truth_table_from_bitset_bennett( func );
}

template<>
std::string html_repr<circuit>( const circuit& circ )
{
  temporary_filename filename( "circuit_pic_%d.qpic" );
  std::string basename( filename.name().begin(), filename.name().end() - 5 );

  write_qpic( circ, filename.name() );

  execute_and_omit( fmt::format( "qpic -f tex {}", filename.name() ) );

  // modify the TeX code
  std::string new_tex_code;
  line_parser( basename + ".tex", {
      {std::regex( "\\\\documentclass\\{article\\}" ), [&new_tex_code]( const std::smatch& s ) {
          new_tex_code += "\\documentclass[dvisvgm]{standalone}\n";
        }},
      {std::regex( "\\\\usepackage\\[pdftex,active,tightpage\\]\\{preview\\}" ), []( const std::smatch& s ) {}},
      {std::regex( "\\\\begin\\{preview\\}" ), []( const std::smatch& s ) {}},
      {std::regex( "\\\\end\\{preview\\}" ), []( const std::smatch& s ) {}},
      {std::regex( "(.*)" ), [&new_tex_code]( const std::smatch& s ) {
          new_tex_code += std::string( s[0u] ) + "\n";
        }}} );

  std::ofstream os( ( basename + ".tex" ).c_str(), std::ofstream::out );
  os << new_tex_code << std::endl;
  os.close();

  execute_and_omit( fmt::format( "latex {}.tex", basename ) );
  execute_and_omit( fmt::format( "dvisvgm -b papersize {}.dvi", basename ) );

  std::remove( fmt::format( "{}.tex", basename ).c_str() );
  std::remove( fmt::format( "{}.dvi", basename ).c_str() );
  std::remove( fmt::format( "{}.out", basename ).c_str() );
  std::remove( fmt::format( "{}.log", basename ).c_str() );

  std::ifstream in( ( basename + ".svg" ).c_str(), std::ifstream::in );
  std::string svg( (std::istreambuf_iterator<char>( in )), std::istreambuf_iterator<char>() );
  in.close();

  std::remove( fmt::format( "{}.svg", basename ).c_str() );

  return svg;
}

/******************************************************************************
 * binary_truth_table                                                         *
 ******************************************************************************/

template<>
std::string to_string<binary_truth_table>( const binary_truth_table& spec )
{
  return fmt::format( "{} inputs, {} outputs", spec.num_inputs(), spec.num_outputs() );
}

template<>
void print<binary_truth_table>( std::ostream& os, const binary_truth_table& spec )
{
  os << spec << std::endl;
}

template<>
binary_truth_table convert<circuit, binary_truth_table>( const circuit& circ )
{
  binary_truth_table spec;
  circuit_to_truth_table( circ, spec, simple_simulation_func() );
  return spec;
}

/******************************************************************************
 * rcbdd                                                                      *
 ******************************************************************************/

template<>
std::string to_string<rcbdd>( const rcbdd& bdd )
{
  return fmt::format( "{} variables, {} nodes", bdd.num_vars(), bdd.chi().nodeCount() );
}

template<>
nlohmann::json log_statistics<rcbdd>( const rcbdd& bdd )
{
  return nlohmann::json({
      {"variables", bdd.num_vars()}
    });
}

template<>
bool can_show<rcbdd>( std::string& extension, command& cmd )
{
  extension = "dot";
  return true;
}

template<>
void show<rcbdd>( std::ostream& out, const rcbdd& bdd, const command& cmd )
{
  const std::string tmp = std::tmpnam( nullptr );
  auto * fd = fopen( tmp.c_str(), "w" );
  
  if ( cmd.is_set( "add" ) )
  {
    bdd.manager().DumpDot( std::vector<ADD>{bdd.chi().Add()}, 0, 0, fd );
  }
  else
  {
    bdd.manager().DumpDot( std::vector<BDD>{bdd.chi()}, 0, 0, fd );
  }

  fclose( fd );

  std::ifstream in( tmp.c_str(), std::ifstream::in );
  std::string line;
  while ( std::getline( in, line ) )
  {
    out << line;
  }
  in.close();

  std::remove( tmp.c_str() );
}

template<>
void print<rcbdd>( std::ostream& os, const rcbdd& bdd )
{
  bdd.print_truth_table();
}

template<>
rcbdd convert<circuit, rcbdd>( const circuit& circ )
{
  rcbdd cf;
  cf.initialize_manager();
  cf.create_variables( circ.lines() );
  cf.set_chi( cf.create_from_circuit( circ ) );

  return cf;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
