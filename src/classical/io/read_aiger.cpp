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

#include "read_aiger.hpp"

#include <classical/utils/aig_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/string_utils.hpp>

#include <boost/algorithm/string/trim.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/assign/std/map.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/counting_range.hpp>

#include <fstream>
#include <sstream>

namespace cirkit
{

using namespace boost::assign;

unsigned aiger_lit2var( const unsigned lit )
{
  return (lit - lit % 2u) / 2u;
}

void read_aiger( aig_graph& aig, const std::string &filename )
{
  std::string comment;
  read_aiger( aig, comment, filename );
}

void read_aiger( aig_graph& aig, std::istream& in )
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

void read_aiger( aig_graph& aig, std::string& comment, std::istream& in )
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

  auto& info = aig_info( aig );

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
    info.inputs += nodes[aiger_lit2var(lit)];
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

    aig_node node_out = nodes[aiger_lit2var(lit_out)];
    aig_node node_in = nodes[aiger_lit2var(lit_in)];

    if ( node_in == 0u )
    {
      info.constant_used = true;
    }

    aig_function in = { node_in, lit_in % 2u == 1u };

    info.cis += node_out;

    info.cos += in;

    info.latch[in] = { node_out, false };
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

    const aig_function f = { nodes[aiger_lit2var(lit)], lit%2 == 1 };
    info.outputs += std::make_pair( f, "" );

    if ( f.node == 0u )
    {
      info.constant_used = true;
    }
  }

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

    aig_node node = nodes[aiger_lit2var(lit_out)];
    aig_node left = nodes[aiger_lit2var(lit_le)];
    aig_node right = nodes[aiger_lit2var(lit_re)];

    aig_edge le = add_edge( node, left, aig ).first;
    boost::get( boost::edge_complement, aig )[le] = lit_le%2;

    aig_edge re = add_edge( node, right, aig ).first;
    boost::get( boost::edge_complement, aig )[re] = lit_re%2;

    if ( lit_le <= 1u || lit_re <= 1u )
    {
      info.constant_used = true;
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
      info.node_names[ info.inputs[id] ] = name;
      break;
    case 'o':
      assert( id < num_outputs && "ID is not in range of outputs" );
      info.outputs[id].second = name;
      break;
    case 'l':
      assert( id < num_latches && "ID is not in range of latches" );
      info.node_names[ info.cis[id] ] = name;
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

  /* fill up symbol table with empty entries */
  for ( const auto& i : index( info.inputs ) )
  {
    if ( info.node_names.find( i.value ) == info.node_names.end() )
    {
      info.node_names += std::make_pair( i.value, "" );
    }
  }
}

unsigned aiger_decode( std::istream& in )
{
  auto i = 0u;
  auto res = 0u;

  while ( true )
  {
    auto c = in.get();
    res |= ( ( c & 0x7F ) << ( 7 * i++ ) );
    if ( !( c & 0x80 ) ) { break; }
  }

  return res;
}

void read_aiger_binary( aig_graph& aig, std::istream& in, bool noopt )
{
  std::string line;

  /* read header */
  std::getline( in, line );
  if ( in.fail() ) { throw "Error: could not read input file (check path and permissions)"; }

  std::vector<std::string> header;
  split_string( header, line, " " );

  if ( header.size() != 6u || header[0u] != "aig" ) { throw "Error: expect 'aig M I L O A' as header"; }

  if ( header[3u] != "0" ) { throw "Error: latches are not supported yet"; }

  const auto num_inputs  = boost::lexical_cast<unsigned>( header[2u] );
  const auto num_outputs = boost::lexical_cast<unsigned>( header[4u] );
  const auto num_ands    = boost::lexical_cast<unsigned>( header[5u] );

  /* create AIG */
  aig_initialize( aig );
  auto& info = aig_info( aig );

  if ( noopt )
  {
    info.enable_strashing = info.enable_local_optimization = false;
  }

  /* store nodes */
  std::vector<aig_function> fs = {aig_get_constant( aig, false )};

  /* create PIs */
  ntimes( num_inputs, [&]() { fs.push_back( aig_create_pi( aig, "" ) ); } );

  /* store output addresses */
  std::vector<unsigned> oids;
  ntimes( num_outputs, [&]() {
      std::getline( in, line );
      boost::trim( line );
      oids += boost::lexical_cast<unsigned>( line );
    } );

  for ( auto i : boost::counting_range( num_inputs + 1u, num_inputs + num_ands + 1u ) )
  {
    const auto g  = i << 1u;
    const auto o1 = g - aiger_decode( in );
    const auto o2 = o1 - aiger_decode( in );

    const auto f = aig_create_and( aig, make_function( fs[o1 / 2u], o1 % 2u ), make_function( fs[o2 / 2u], o2 % 2u ) );
    assert( fs.size() == i );
    assert( !noopt || f.node == i );

    fs.push_back( f );
  }

  for ( const auto& oid : oids )
  {
    aig_create_po( aig, make_function( fs[oid / 2u], oid % 2u ), "" );
  }

  while ( std::getline( in, line ) )
  {
    if ( line.size() != 0u && line[0] == 'c' ) { break; }
    if ( line.size() == 0u || ( line[0] != 'i' && line[0] != 'o' ) ) { continue; }

    std::vector<std::string> list;
    split_string( list, line, " " );

    const auto pos = boost::lexical_cast<unsigned>( list[0u].substr( 1u ) );
    std::string name = list.size() == 1u ? "unknown" : list[1u];

    if ( list[0][0] == 'i' )
    {
      info.node_names[info.inputs[pos]] = name;
    }
    else if ( list[0][0] == 'o' )
    {
      info.outputs[pos].second = name;
    }
  }
}

void read_aiger_binary( aig_graph& aig, const std::string& filename, bool noopt )
{
  std::ifstream in( filename.c_str(), std::ifstream::in );
  read_aiger_binary( aig, in, noopt );

  aig_info( aig ).model_name = boost::filesystem::path( filename ).stem().string();
  in.close();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
