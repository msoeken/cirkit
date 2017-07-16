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

#include "stores.hpp"

#include <cstdlib>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <core/utils/string_utils.hpp>
#include <core/utils/system_utils.hpp>
#include <core/utils/temporary_filename.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <reversible/functions/circuit_from_string.hpp>
#include <reversible/functions/circuit_to_aig.hpp>
#include <reversible/functions/circuit_to_truth_table.hpp>
#include <reversible/functions/permutation_to_truth_table.hpp>
#include <reversible/functions/truth_table_from_bitset.hpp>
#include <reversible/io/create_image.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/io/print_statistics.hpp>
#include <reversible/io/read_qc.hpp>
#include <reversible/io/read_realization.hpp>
#include <reversible/io/read_specification.hpp>
#include <reversible/io/write_liquid.hpp>
#include <reversible/io/write_numpy.hpp>
#include <reversible/io/write_pla.hpp>
#include <reversible/io/write_projectq.hpp>
#include <reversible/io/write_qc.hpp>
#include <reversible/io/write_qcode.hpp>
#include <reversible/io/write_qpic.hpp>
#include <reversible/io/write_quipper.hpp>
#include <reversible/io/write_realization.hpp>
#include <reversible/io/write_specification.hpp>
#include <reversible/simulation/simple_simulation.hpp>
#include <reversible/utils/circuit_utils.hpp>
#include <reversible/utils/costs.hpp>

namespace alice
{

using namespace cirkit;

/******************************************************************************
 * circuit                                                                    *
 ******************************************************************************/

template<>
std::string store_entry_to_string<circuit>( const circuit& circ )
{
  return ( boost::format( "%d lines, %d gates" ) % circ.lines() % circ.num_gates() ).str();
}

template<>
void print_store_entry<circuit>( std::ostream& os, const circuit& circ )
{
  os << circ << std::endl;
}

template<>
void print_store_entry_statistics<circuit>( std::ostream& os, const circuit& circ )
{
  print_statistics_settings settings;
  settings.main_template = "%1$sLines:        %2$d\nGates:        %3$d\nT-count:      %6$d\nLogic qubits: %8$d\n";
  print_statistics( os, circ, -1.0, settings );
}

template<>
command::log_opt_t log_store_entry_statistics<circuit>( const circuit& circ )
{
  return command::log_opt_t({
      {"gates", static_cast<int>( circ.num_gates() )},
      {"lines", static_cast<int>( circ.lines() )},
      {"tdepth", static_cast<unsigned>( costs( circ, costs_by_gate_func( t_depth_costs() ) ) )},
      {"tcount", static_cast<unsigned>( costs( circ, costs_by_gate_func( t_costs() ) ) )},
      {"ncv",    static_cast<unsigned>( costs( circ, costs_by_gate_func( ncv_quantum_costs() ) ) )},
      {"qubits", number_of_qubits( circ )},
      {"depth", static_cast<unsigned>( costs( circ, costs_by_circuit_func( depth_costs() ) ) )}
    });
}

show_store_entry<circuit>::show_store_entry( const command& cmd )
{
}

bool show_store_entry<circuit>::operator()( circuit& circ, const std::string& dotname, const command& cmd )
{
  temporary_filename qpic_filename( "test-%d.qpic" );
  temporary_filename png_filename( "test-%d.png" );
  write_qpic( circ, qpic_filename.name() );
  system( boost::str( boost::format( "qpic -o %s -f png %s" ) % png_filename.name() % qpic_filename.name() ).c_str() );
  system( boost::str( boost::format( "convert %1% -background white -alpha remove -resize x300 %1%" ) % png_filename.name() ).c_str() );
  system( boost::str( boost::format( "imgcat %s" ) % png_filename.name() ).c_str() );
  return false;
}

command::command::log_opt_t show_store_entry<circuit>::log() const
{
  return boost::none;
}

template<>
aig_graph store_convert<circuit, aig_graph>( const circuit& circ )
{
  return circuit_to_aig( circ );
}

template<>
binary_truth_table store_convert<expression_t::ptr, binary_truth_table>( const expression_t::ptr& expr )
{
  return truth_table_from_bitset_bennett( tt_from_expression( expr ) );
}

template<>
binary_truth_table store_convert<tt, binary_truth_table>( const tt& func )
{
  return truth_table_from_bitset_bennett( func );
}

template<>
bool store_can_write_io_type<circuit, io_qpic_tag_t>( command& cmd )
{
  cmd.opts.add_options()
    ( "print_index,i", "prints index below each gate" )
    ;
  return true;
}

template<>
void store_write_io_type<circuit, io_qpic_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
{
  auto settings = std::make_shared<properties>();
  settings->set( "print_index", cmd.is_set( "print_index" ) );
  write_qpic( circ, filename, settings );
}

template<>
bool store_can_write_io_type<circuit, io_quipper_tag_t>( command& cmd )
{
  cmd.opts.add_options()
    ( "ascii,a", "write ASCII instead of Haskell program" )
    ;

  return true;
}

template<>
void store_write_io_type<circuit, io_quipper_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
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
bool store_can_write_io_type<circuit, io_liquid_tag_t>( command& cmd )
{
  cmd.opts.add_options()
    ( "dump,d", "write dump statement into script" )
    ;

  return true;
}

template<>
void store_write_io_type<circuit, io_liquid_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
{
  auto settings = std::make_shared<properties>();
  settings->set( "dump_statement", cmd.is_set( "dump" ) );

  write_liquid( circ, filename, settings );
}

template<>
bool store_can_read_io_type<circuit, io_real_tag_t>( command& cmd )
{
  cmd.opts.add_options()
    ( "string,s", boost::program_options::value<std::string>(), "read from string (e.g. t3 a b c, t2 a b, t1 a, f3 a b c)" )
    ;
  return true;
}

template<>
circuit store_read_io_type<circuit, io_real_tag_t>( const std::string& filename, const command& cmd )
{
  if ( cmd.is_set( "string" ) )
  {
    return circuit_from_string( cmd.vm["string"].as<std::string>() );
  }
  else
  {
    circuit circ;
    read_realization( circ, filename );
    return circ;
  }
}

template<>
bool store_can_write_io_type<circuit, io_real_tag_t>( command& cmd )
{
  cmd.opts.add_options()
    ( "string,s", "write to string (which can be read with read_real -s)" )
    ;
  return true;
}

template<>
void store_write_io_type<circuit, io_real_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
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
bool store_can_write_io_type<circuit, io_tikz_tag_t>( command& cmd )
{
  boost::program_options::options_description circuit_options( "Circuit options" );

  circuit_options.add_options()
    ( "standalone", "Surround Tikz code with LaTeX template to compile" )
    ( "hideio",     "Don't print I/O" )
    ;

  cmd.opts.add( circuit_options );

  return true;
}

template<>
void store_write_io_type<circuit, io_tikz_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
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
circuit store_read_io_type<circuit, io_qc_tag_t>( const std::string& filename, const command& cmd )
{
  return read_qc( filename );
}

template<>
bool store_can_write_io_type<circuit, io_qc_tag_t>( command& cmd )
{
  cmd.opts.add_options()
    ( "iqc", "write IQC compliant files" )
    ;
  return true;
}

template<>
void store_write_io_type<circuit, io_qc_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
{
  write_qc( circ, filename, cmd.is_set( "iqc" ) );
}

template<>
void store_write_io_type<circuit, io_qcode_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
{
  write_qcode( circ, filename );
}

template<>
void store_write_io_type<circuit, io_numpy_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
{
  write_numpy( circ, filename );
}

template<>
void store_write_io_type<circuit, io_projectq_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
{
  write_projectq( circ, filename );
}

template<>
std::string store_repr_html<circuit>( const circuit& circ )
{
  temporary_filename filename( "circuit_pic_%d.qpic" );
  std::string basename( filename.name().begin(), filename.name().end() - 5 );

  write_qpic( circ, filename.name() );

  execute_and_omit( boost::str( boost::format( "qpic -f tex %s" ) % filename.name() ) );

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

  execute_and_omit( boost::str( boost::format( "latex %s.tex" ) % basename ) );
  execute_and_omit( boost::str( boost::format( "dvisvgm -b papersize %s.dvi" ) % basename ) );

  boost::filesystem::remove( basename + ".tex" );
  boost::filesystem::remove( basename + ".dvi" );
  boost::filesystem::remove( basename + ".out" );
  boost::filesystem::remove( basename + ".log" );

  std::ifstream in( ( basename + ".svg" ).c_str(), std::ifstream::in );
  std::string svg( (std::istreambuf_iterator<char>( in )), std::istreambuf_iterator<char>() );
  in.close();
  boost::filesystem::remove( basename + ".svg" );

  return svg;
}

/******************************************************************************
 * binary_truth_table                                                         *
 ******************************************************************************/

template<>
std::string store_entry_to_string<binary_truth_table>( const binary_truth_table& spec )
{
  return ( boost::format( "%d inputs, %d outputs" ) % spec.num_inputs() % spec.num_outputs() ).str();
}

template<>
void print_store_entry<binary_truth_table>( std::ostream& os, const binary_truth_table& spec )
{
  os << spec << std::endl;
}

template<>
binary_truth_table store_convert<circuit, binary_truth_table>( const circuit& circ )
{
  binary_truth_table spec;
  circuit_to_truth_table( circ, spec, simple_simulation_func() );
  return spec;
}

template<>
bool store_can_read_io_type<binary_truth_table, io_spec_tag_t>( command& cmd )
{
  cmd.opts.add_options()
    ( "permutation,p", boost::program_options::value<std::string>(), "create spec from permutation (starts with 0, space separated)" )
    ;
  return true;
}

template<>
binary_truth_table store_read_io_type<binary_truth_table, io_spec_tag_t>( const std::string& filename, const command& cmd )
{
  if ( cmd.is_set( "permutation" ) )
  {
    std::vector<unsigned> perm;
    parse_string_list( perm, cmd.vm["permutation"].as<std::string>() );

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
void store_write_io_type<binary_truth_table, io_spec_tag_t>( const binary_truth_table& spec, const std::string& filename, const command& cmd )
{
  write_specification( spec, filename );
}

template<>
void store_write_io_type<binary_truth_table, io_pla_tag_t>( const binary_truth_table& spec, const std::string& filename, const command& cmd )
{
  write_pla( spec, filename );
}

/******************************************************************************
 * rcbdd                                                                      *
 ******************************************************************************/

template<>
std::string store_entry_to_string<rcbdd>( const rcbdd& bdd )
{
  return ( boost::format( "%d variables, %d nodes" ) % bdd.num_vars() % bdd.chi().nodeCount() ).str();
}

template<>
command::log_opt_t log_store_entry_statistics<rcbdd>( const rcbdd& bdd )
{
  return command::log_opt_t({
      {"variables", bdd.num_vars()}
    });
}

show_store_entry<rcbdd>::show_store_entry( const command& cmd )
{
}

bool show_store_entry<rcbdd>::operator()( rcbdd& bdd,
                                          const std::string& dotname,
                                          const command& cmd )
{
  using namespace std::placeholders;

  auto * fd = fopen( dotname.c_str(), "w" );

  if ( cmd.is_set( "add" ) )
  {
    bdd.manager().DumpDot( std::vector<ADD>{bdd.chi().Add()}, 0, 0, fd );
  }
  else
  {
    bdd.manager().DumpDot( std::vector<BDD>{bdd.chi()}, 0, 0, fd );
  }

  fclose( fd );

  return true;
}

command::log_opt_t show_store_entry<rcbdd>::log() const
{
  return boost::none;
}

template<>
void print_store_entry<rcbdd>( std::ostream& os, const rcbdd& bdd )
{
  bdd.print_truth_table();
}

template<>
rcbdd store_convert<circuit, rcbdd>( const circuit& circ )
{
  rcbdd cf;
  cf.initialize_manager();
  cf.create_variables( circ.lines() );
  cf.set_chi( cf.create_from_circuit( circ ) );

  return cf;
}

template<>
bool store_can_write_io_type<rcbdd, io_pla_tag_t>( command& cmd )
{
  cmd.opts.add_options()
    ( "full", "also write constants and garbage (only for RCBDD)" )
    ;
  return true;
}

template<>
void store_write_io_type<rcbdd, io_pla_tag_t>( const rcbdd& bdd, const std::string& filename, const command& cmd )
{
  bdd.write_pla( filename, cmd.is_set( "full" ) );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
