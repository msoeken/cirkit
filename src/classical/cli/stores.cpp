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

#include <sstream>

#include <boost/format.hpp>

#include <core/graph/depth.hpp>
#include <core/utils/range_utils.hpp>

#include <classical/functions/aig_from_truth_table.hpp>
#include <classical/functions/compute_levels.hpp>

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
std::string store_entry_to_string<aig_graph>( const aig_graph& aig )
{
  const auto& info = aig_info( aig );
  const auto& name = info.model_name;
  return boost::str( boost::format( "%s i/o = %d/%d" ) % ( name.empty() ? "(unnamed)" : name ) % info.inputs.size() % info.outputs.size() );
}

show_store_entry<aig_graph>::show_store_entry( program_options& opts )
{
  boost::program_options::options_description aig_options( "AIG options" );

  aig_options.add_options()
    ( "levels", boost::program_options::value<unsigned>()->default_value( 0u ), "Compute and annotate levels for dot\n0: don't compute\n1: push to inputs\n2: push to outputs" )
    ;

  opts.add( aig_options );
}

bool show_store_entry<aig_graph>::operator()( aig_graph& aig, const std::string& dotname, const program_options& opts, const properties::ptr& settings )
{
  const auto levels = opts.variables()["levels"].as<unsigned>();
  if ( levels > 0u )
  {
    auto cl_settings = std::make_shared<properties>();
    cl_settings->set( "verbose", settings->get<bool>( "verbose" ) );
    cl_settings->set( "push_to_outputs", levels == 2u );
    auto annotation = get( boost::vertex_annotation, aig );

    const auto vertex_levels = compute_levels( aig, cl_settings );
    for ( const auto& p : vertex_levels )
    {
      annotation[p.first]["level"] = std::to_string( p.second );
    }

    settings->set( "vertex_levels", boost::optional<std::map<aig_node, unsigned>>( vertex_levels ) );
  }

  write_dot( aig, dotname, settings );

  return true;
}

command_log_opt_t show_store_entry<aig_graph>::log() const
{
  return boost::none;
}

template<>
void print_store_entry_statistics<aig_graph>( std::ostream& os, const aig_graph& aig )
{
  aig_print_stats( aig );
}

template<>
command_log_opt_t log_store_entry_statistics<aig_graph>( const aig_graph& aig )
{
  const auto& info = aig_info( aig );

  std::vector<aig_node> outputs;
  for ( const auto& output : info.outputs )
  {
    outputs += output.first.node;
  }

  std::vector<unsigned> depths;
  const auto depth = compute_depth( aig, outputs, depths );

  return command_log_opt_t({
      {"inputs", static_cast<int>( info.inputs.size() )},
      {"outputs", static_cast<int>( info.outputs.size() )},
      {"size", static_cast<int>( boost::num_vertices( aig ) - info.inputs.size() - 1u )},
      {"depth", static_cast<int>( depth )}});
}

template<>
aig_graph store_convert<tt, aig_graph>( const tt& t )
{
  return aig_from_truth_table( t );
}

template<>
std::string store_entry_to_string<counterexample_t>( const counterexample_t& cex )
{
  std::stringstream os;
  os << cex;
  return os.str();
}

template<>
std::string store_entry_to_string<simple_fanout_graph_t>( const simple_fanout_graph_t& nl )
{
  return "";
}

template<>
std::string store_entry_to_string<std::vector<aig_node>>( const std::vector<aig_node>& g )
{
  return ( boost::format( "{ %s }" ) % any_join( g, ", " ) ).str();
}

template<>
void print_store_entry<std::vector<aig_node>>( std::ostream& os, const std::vector<aig_node>& g )
{
  os << boost::format( "{ %s }" ) % any_join( g, ", " ) << std::endl;
}

template<>
std::string store_entry_to_string<tt>( const tt& t )
{
  std::stringstream os;
  os << t;
  return os.str();
}

template<>
void print_store_entry<tt>( std::ostream& os, const tt& t )
{
  os << t << std::endl;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
