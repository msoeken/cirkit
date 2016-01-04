/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

#include <boost/format.hpp>

#include <reversible/functions/circuit_to_aig.hpp>
#include <reversible/io/create_image.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/io/print_statistics.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
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
command_log_opt_t log_store_entry_statistics<circuit>( const circuit& circ )
{
  return command_log_opt_t({
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
std::string store_entry_to_string<binary_truth_table>( const binary_truth_table& spec )
{
  return ( boost::format( "%d inputs, %d outputs" ) % spec.num_inputs() % spec.num_outputs() ).str();
}

show_store_entry<circuit>::show_store_entry( program_options& opts )
{
}

bool show_store_entry<circuit>::operator()( circuit& circ,
                                            const std::string& dotname,
                                            const program_options& opts,
                                            const properties::ptr& settings )
{
  create_tikz_settings ct_settings;
  create_image( dotname, circ, ct_settings );

  return false; /* don't open dot viewer */
}

template<>
void print_store_entry<binary_truth_table>( std::ostream& os, const binary_truth_table& spec )
{
  os << spec << std::endl;
}

template<>
std::string store_entry_to_string<rcbdd>( const rcbdd& bdd )
{
  return ( boost::format( "%d variables, %d nodes" ) % bdd.num_vars() % bdd.chi().nodeCount() ).str();
}

show_store_entry<rcbdd>::show_store_entry( program_options& opts )
{
}

bool show_store_entry<rcbdd>::operator()( rcbdd& bdd,
                                          const std::string& dotname,
                                          const program_options& opts,
                                          const properties::ptr& settings )
{
  using namespace std::placeholders;

  auto * fd = fopen( dotname.c_str(), "w" );

  if ( opts.is_set( "add" ) )
  {
    bdd.manager().DumpDot( {bdd.chi().Add()}, 0, 0, fd );
  }
  else
  {
    bdd.manager().DumpDot( {bdd.chi()}, 0, 0, fd );
  }

  fclose( fd );

  return true;
}

template<>
void print_store_entry<rcbdd>( std::ostream& os, const rcbdd& bdd )
{
  bdd.print_truth_table();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
