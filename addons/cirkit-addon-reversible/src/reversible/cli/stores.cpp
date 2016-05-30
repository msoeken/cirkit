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

#include "stores.hpp"

#include <vector>

#include <boost/format.hpp>

#include <core/utils/string_utils.hpp>
#include <reversible/functions/circuit_from_string.hpp>
#include <reversible/functions/circuit_to_aig.hpp>
#include <reversible/functions/circuit_to_truth_table.hpp>
#include <reversible/functions/permutation_to_truth_table.hpp>
#include <reversible/io/create_image.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/io/print_statistics.hpp>
#include <reversible/io/read_realization.hpp>
#include <reversible/io/read_specification.hpp>
#include <reversible/io/write_pla.hpp>
#include <reversible/io/write_qpic.hpp>
#include <reversible/io/write_quipper.hpp>
#include <reversible/io/write_realization.hpp>
#include <reversible/io/write_specification.hpp>
#include <reversible/simulation/simple_simulation.hpp>

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
  print_statistics( os, circ );
}

template<>
command::log_opt_t log_store_entry_statistics<circuit>( const circuit& circ )
{
  return command::log_opt_t({
      {"gates", static_cast<int>( circ.num_gates() )},
      {"lines", static_cast<int>( circ.lines() )}
    });
}

template<>
aig_graph store_convert<circuit, aig_graph>( const circuit& circ )
{
  return circuit_to_aig( circ );
}

template<>
void store_write_io_type<circuit, io_qpic_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
{
  write_qpic( circ, filename );
}

template<>
bool store_can_write_io_type<circuit, io_quipper_tag_t>( command& cmd )
{
  cmd.opts.add_options()
    ( "ascii,a", "Write ASCII instead of Haskell program" )
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
void store_write_io_type<circuit, io_real_tag_t>( const circuit& circ, const std::string& filename, const command& cmd )
{
  write_realization( circ, filename );
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

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
