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

#include "read_aiger.hpp"

#include <classical/utils/aig_utils.hpp>

#include <boost/assign/std/vector.hpp>
#include <boost/filesystem.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

namespace cirkit
{

using namespace boost::assign;

inline unsigned lit2var( const unsigned lit )
{
  return (lit - lit % 2) / 2;
}

void read_aiger( aig_graph& aig, const std::string &filename )
{
  std::string comment;
  read_aiger( aig, comment, filename );
}

void read_aiger( aig_graph& aig, std::ifstream& in )
{
  std::string comment;
  read_aiger( aig, comment, in );
}

void read_aiger( aig_graph& aig, std::string& comment, const std::string &filename )
{
  std::ifstream is( filename.c_str() );
  read_aiger( aig, comment, is );
  auto& info = aig_info( aig );
  info.model_name = boost::filesystem::path( filename ).stem().string();
  is.close();
}

void read_aiger( aig_graph& aig, std::string& comment, std::ifstream& in )
{
  /* read AIGER header */
  std::string line;
  std::getline(in, line);
  if ( in.fail() )
    throw "Error: could not read input file (check path and permissions)";

  /* parse AIGER header */
  std::istringstream is(line);
  std::string sig;
  is >> sig;
  if ( sig != "aag" )
    throw "Error: expected ``aag'' at the beginning of the header";

  unsigned num_ids;
  is >> num_ids;
  if ( is.fail() )
    throw "Error: could not read number of IDs";

  unsigned num_inputs;
  is >> num_inputs;
  if ( is.fail() )
    throw "Error: could not read the number of inputs";

  unsigned num_latches;
  is >> num_latches;
  if ( is.fail() )
    throw "Error: could not read the number of latches";

  unsigned num_outputs;
  is >> num_outputs;
  if ( is.fail() )
    throw "Error: could not read the number of outputs";

  unsigned num_gates;
  is >> num_gates;
  if ( is.fail() )
    throw "Error: could not read the number of gates";

  if ( num_ids != num_inputs + num_latches + num_gates )
    throw "Error: broken AAG header";

  // std::cout << "aag "
  //           << num_ids << ' '
  //           << num_inputs << ' '
  //           << num_latches << ' '
  //           << num_outputs << ' '
  //           << num_gates << '\n';

  /* create all AIG nodes in advance */
  aig_initialize( aig );
  std::vector< aig_node > nodes(num_ids+1);
  for ( unsigned id = 1u; id < num_ids+1; ++id )
  {
    aig_node node = add_vertex( aig );
    boost::get( boost::vertex_name, aig )[node] = 2u*(id);
    nodes[id] = node;
  }

  /* read inputs and mark them in AIG */
  for ( unsigned u = 0u; u < num_inputs; ++u )
  {
    std::getline(in, line);
    if ( in.fail() )
      throw "Error: could not read input definition";

    unsigned lit;
    std::istringstream is(line);
    is >> lit;
    if ( is.fail() )
      throw "Error: could not parse input definition";
    if ( lit%2 != 0u )
      throw "Error: negated inputs are not permitted in definition";

    // std::cout << lit << '\n';
    boost::get_property( aig, boost::graph_name ).inputs += nodes[lit2var(lit)];
  }

  /* read latches */
  for ( unsigned u = 0u; u < num_latches; ++u )
  {
    std::getline(in, line);
    if ( in.fail() )
      throw "Error: could not read latch definition";

    unsigned lit_out, lit_in;
    std::istringstream is(line);
    is >> lit_out;
    if ( is.fail() )
      throw "Error: could not parse latch definition";
    if ( lit_out%2 != 0u )
      throw "Error: negated latch outputs are not permitted in definition";
    is >> lit_in;
    if ( is.fail() )
      throw "Error: could not parse latch definition";

    // std::cout << lit_out << ' '  << lit_in << '\n';

    aig_node node_out = nodes[lit2var(lit_out)];
    aig_node node_in = nodes[lit2var(lit_in)];

    if ( node_in == 0u )
    {
      boost::get_property( aig, boost::graph_name ).constant_used = true;
    }

    auto in = std::make_pair( node_in, lit_in%2 );

    boost::get_property( aig, boost::graph_name ).cis += node_out;

    boost::get_property( aig, boost::graph_name ).cos += in;

    boost::get_property( aig, boost::graph_name ).latch[in] = std::make_pair( node_out, false );
  }

  /* read outputs and mark them in AIG */
  for ( unsigned u = 0u; u < num_outputs; ++u )
  {
    std::getline(in, line);
    if ( in.fail() )
      throw "Error: could not read output definition";

    unsigned lit;
    std::istringstream is(line);
    is >> lit;
    if ( is.fail() )
      throw "Error: could not parse output definition";

    // std::cout << lit << '\n';

    boost::get_property( aig, boost::graph_name ).outputs +=
      std::make_pair( std::make_pair( nodes[lit2var(lit)], lit%2 ), "" );
  }

  auto& graph_info = boost::get_property( aig, boost::graph_name );

  /* read and gates and create edges in AIG */
  for ( unsigned u = 0u; u < num_gates; ++u )
  {
    std::getline(in, line);
    if ( in.fail() )
      throw "Error: could not read gate definition";

    unsigned lit_out, lit_le, lit_re;
    std::istringstream is(line);
    is >> lit_out;
    if ( is.fail() )
      throw "Error: could not parse gate definition";
    if ( lit_out%2 != 0 )
      throw "Error: negated gates are not permitted in definition";
    is >> lit_le;
    if ( is.fail() )
      throw "Error: could not parse gate definition";
    is >> lit_re;
    if ( is.fail() )
      throw "Error: could not parse gate definition";

    // std::cout << lit_out << ' ' << lit_le << ' ' << lit_re << '\n';

    aig_node node = nodes[lit2var(lit_out)];
    aig_node left = nodes[lit2var(lit_le)];
    aig_node right = nodes[lit2var(lit_re)];

    aig_edge le = add_edge( node, left, aig ).first;
    boost::get( boost::edge_complement, aig )[le] = lit_le%2;

    aig_edge re = add_edge( node, right, aig ).first;
    boost::get( boost::edge_complement, aig )[re] = lit_re%2;

    if ( left <= 1u || right <= 1u )
    {
      graph_info.constant_used = true;
    }
  }

  /* read optional symbol table and assign names to nodes */
  for ( ;; )
  {
    std::getline(in, line);

    // no optional names or comment section
    if ( in.fail() ) break;

    std::istringstream is(line);

    char c;
    is >> c;
    if ( is.fail() )
      throw "Error: could not read symbol table";

    // no optional names but comment section
    if ( c == 'c' ) break;

    // note that interleaved input, output, and latch names are allowed
    if ( c != 'i' && c != 'o' && c != 'l')
      throw "Error: unsupported symbol table entry";

    unsigned id;
    is >> id;
    if ( is.fail() )
      throw "Error: could not parse symbol table (id)";

    std::string name;
    is >> name;
    if ( is.fail() )
      throw "Error: could not parse symbol table (name)";

    switch (c)
    {
    case 'i':
      assert( id < num_inputs && "ID is not in range of inputs" );
      graph_info.node_names[ graph_info.inputs[id] ] = name;
      break;
    case 'o':
      assert( id < num_outputs && "ID is not in range of outputs" );
      graph_info.outputs[id].second = name;
      break;
    case 'l':
      assert( id < num_latches && "ID is not in range of latches" );
      graph_info.node_names[ graph_info.cis[id] ] = name;
      break;
    default:
      // unreachable
      throw "Error: unsupported symbol table entry";
    }

    // std::cout << c << id << ' ' << name << '\n';
  }

  /* read comment and ignore it */
  // std::cout << 'c' << '\n';
  for ( ;; )
  {
    std::getline(in, line);

    // end of comment section
    if ( in.fail() ) break;

    // std::cout << line << '\n';
    comment += line + '\n';;
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
