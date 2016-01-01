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

#include "simgraph.hpp"

#include <fstream>

#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <boost/range/algorithm.hpp>

#include <core/cli/rules.hpp>
#include <core/utils/program_options.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/string_utils.hpp>
#include <classical/functions/simulation_graph.hpp>

using namespace boost::assign;
using namespace boost::program_options;

using boost::format;

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

simgraph_command::simgraph_command( const environment::ptr& env ) : aig_base_command( env, "Creates simulation graphs" )
{
  opts.add_options()
    ( "vectors",      value_with_default( &vectors ),    "Simulation vectors (comma separated):\nah: all-hot\n1h: one-hot\n2h: two-hot\nac: all-cold\n1c: one-cold\n2c: two-cold" )
    ( "dotname",      value( &dotname ),                 "If set, simulation file is written to DOT file" )
    ( "signatures,s", value_with_default( &signatures ), "Maximum arity of simulation signatures" )
    ( "patternname",  value( &patternname ),             "If filename is given, simulation vectors are written to this file" )
    ;
  be_verbose();
}

command::rules_t simgraph_command::validity_rules() const
{
  return { has_store_element<aig_graph>( env ), { [&]() { return !vectors.empty(); }, "no simulation vector specified" } };
}

bool simgraph_command::execute()
{
  std::vector<unsigned> types;
  foreach_string( vectors, ",", [&]( const std::string& s ) {
      if      ( s == "ah" ) types += 0u;
      else if ( s == "1h" ) types += 3u;
      else if ( s == "2h" ) types += 5u;
      else if ( s == "ac" ) types += 1u;
      else if ( s == "1c" ) types += 2u;
      else if ( s == "2c" ) types += 4u;
    } );

  auto settings = make_settings();
  settings->set( "simulation_signatures", signatures ? boost::optional<unsigned>( signatures ) : boost::optional<unsigned>() );
  if ( opts.is_set( "dotname" ) )
  {
    settings->set( "dotname", dotname );
  }
  statistics = std::make_shared<properties>();

  const auto graph = create_simulation_graph( aig(), types, settings, statistics );

  std::cout << format( "[i] create_simulation_graph:  %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl
            << format( "[i] - labeling time:          %.2f secs" ) % statistics->get<double>( "labeling_runtime" ) << std::endl
            << format( "[i] - vertices:               %d" ) % boost::num_vertices( graph ) << std::endl
            << format( "[i] - edges:                  %d" ) % boost::num_edges( graph ) << std::endl;

  if ( opts.is_set( "signatures" ) && signatures )
  {
    const auto& _info = info();
    const auto& meta  = boost::get_property( graph, boost::graph_meta );
    const auto offset = meta.num_inputs + meta.num_vectors;
    const auto& sigs  = boost::get( boost::vertex_simulation_signature, graph );

    std::cout << "[i] simulation signatures:" << std::endl;
    for ( auto i = 0u; i < _info.outputs.size(); ++i )
    {
      std::cout << format( "[i] %s : %s" ) % _info.outputs.at( i ).second % any_join( *sigs[offset + i], " " ) << std::endl;
    }
  }

  if ( opts.is_set( "patternname" ) )
  {
    write_pattern_file();
  }

  return true;
}

void simgraph_command::write_pattern_file()
{
  std::ofstream os( patternname.c_str(), std::ofstream::out );

  os << format( "PatternList %s %s" ) % info().model_name % vectors << std::endl << std::endl;

  os << format( "PI %d" ) % info().inputs.size() << std::endl;

  std::vector<std::string> input_names;
  for ( const auto& input : info().inputs )
  {
    input_names += info().node_names.at( input );
  }

  os << any_join( input_names, " " ) << std::endl << std::endl;

  const auto& patterns = statistics->get<std::vector<boost::dynamic_bitset<>>>( "vectors" );
  os << format( "Patterns %d" ) % patterns.size() << std::endl;

  for ( const auto& p : patterns )
  {
    std::string s;
    boost::to_string( p, s );
    boost::reverse( s );
    os << s << std::endl << std::endl;
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
