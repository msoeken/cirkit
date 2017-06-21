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

#include "mig_verilog.hpp"

#include <chrono>
#include <ctime>
#include <vector>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/utils/graph_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/string_utils.hpp>
#include <classical/mig/mig_utils.hpp>

using namespace boost::assign;

using boost::format;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

struct mighty_operation_t
{
  enum class opcode { _and, _or, _maj };

  unsigned                               target;
  opcode                                 operation;
  std::vector<std::pair<unsigned, bool>> operands; /* second is complemented flag */
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

mig_graph read_mighty_verilog( const std::string& filename,
                               const properties::ptr& settings,
                               const properties::ptr& statistics )
{
  mig_graph mig;
  mig_initialize( mig );

  auto& info = mig_info( mig );

  std::map<std::string, unsigned>                  name_to_function;
  std::vector<std::string>                         output_names;
  std::vector<std::pair<std::string, std::string>> outputs;
  auto                                             num_inputs = 0u;
  auto                                             num_wires = 0u;
  std::vector<mig_function>                        functions;
  std::vector<mighty_operation_t>                  wires;

  digraph_t<>                                      wire_dependency_graph;

  auto get_operand = [&]( const std::string& s ) {
    return ( s[0] == '~' ) ? std::make_pair( name_to_function.at( s.substr( 1u ) ), true ) : std::make_pair( name_to_function.at( s ), false );
  };

  line_parser( filename,
               {
                 { std::regex( "^module (.*) \\($" ), [&]( const std::smatch& m ) {
                     info.model_name = m[1];
                   } },
                 { std::regex( "^input (.*);" ), [&]( const std::smatch& m ) {
                     auto sinputs = std::string( m[1] );
                     std::vector<std::string> inputs;
                     boost::split( inputs, sinputs, boost::is_any_of( "," ), boost::algorithm::token_compress_on );

                     num_inputs = inputs.size();

                     for ( auto& input : inputs )
                     {
                       boost::trim( input );
                       name_to_function.insert( std::make_pair( input, functions.size() ) );
                       functions.push_back( mig_create_pi( mig, input ) );
                     }
                   } },
                 { std::regex( "^output (.*);" ), [&]( const std::smatch& m ) {
                     const auto soutputs = std::string( m[1] );
                     boost::split( output_names, soutputs, boost::is_any_of( "," ), boost::algorithm::token_compress_on );
                     boost::for_each( output_names, []( std::string& s ) { boost::trim( s ); } );
                   } },
                 { std::regex( "^wire (.*);" ), [&]( const std::smatch& m ) {
                     auto swires = std::string( m[1] );
                     num_wires = boost::count( swires, ',' );

                     ntimes( num_wires, [&]() { add_vertex( wire_dependency_graph ); } );

                     const auto s = functions.size();
                     functions.resize( s + num_wires, {boost::graph_traits<mig_graph>::null_vertex(), false} );
                     for ( auto i = 0u; i < num_wires; ++i )
                     {
                       name_to_function.insert( {boost::str( boost::format( "w%d" ) % i ), s + i} );
                     }
                   } },
                 { std::regex( "^assign w(\\d+) = (.*);" ), [&]( const std::smatch& m ) {
                     const auto id = boost::lexical_cast<unsigned>( m[1] );

                     std::smatch match;
                     mighty_operation_t f;

                     std::vector<std::string> wire_operands;

                     const auto expr = std::string( m[2] );

                     if ( std::regex_search( expr, match, std::regex( "^([^ ]+) & ([^ ]+)$" ) ) )
                     {
                       auto sm0 = std::string( match[1] );
                       auto sm1 = std::string( match[2] );

                       f.operation = mighty_operation_t::opcode::_and;
                       f.operands += get_operand( sm0 ),get_operand( sm1 );

                       if ( sm0[0] == '~' ) { sm0 = sm0.substr( 1u ); }
                       if ( sm1[0] == '~' ) { sm1 = sm1.substr( 1u ); }

                       if ( sm0[0] == 'w' ) { wire_operands += sm0; }
                       if ( sm1[0] == 'w' ) { wire_operands += sm1; }
                     }
                     else if ( std::regex_search( expr, match, std::regex( "^([^ ]+) \\| ([^ ]+)$" ) ) )
                     {
                       auto sm0 = std::string( match[1] );
                       auto sm1 = std::string( match[2] );

                       f.operation = mighty_operation_t::opcode::_or;
                       f.operands += get_operand( sm0 ),get_operand( sm1 );

                       if ( sm0[0] == '~' ) { sm0 = sm0.substr( 1u ); }
                       if ( sm1[0] == '~' ) { sm1 = sm1.substr( 1u ); }

                       if ( sm0[0] == 'w' ) { wire_operands += sm0; }
                       if ( sm1[0] == 'w' ) { wire_operands += sm1; }
                     }
                     else if ( std::regex_search( expr, match, std::regex( "^\\(([^ ]+) & ([^ ]+)\\) \\| \\(([^ ]+) & ([^ ]+)\\) \\| \\(([^ ]+) & ([^ ]+)\\)$" ) ) )
                     {
                       /* no assertions here, we assume that operands are used in the correct way */
                       std::string sm0 = match[1], sm1 = match[2];
                       std::string sm2 = ( match[3] == sm0 || match[3] == sm1 ) ? match[4] : match[3];

                       f.operation = mighty_operation_t::opcode::_maj;
                       f.operands += get_operand( sm0 ),get_operand( sm1 ),get_operand( sm2 );

                       if ( sm0[0] == '~' ) { sm0 = sm0.substr( 1u ); }
                       if ( sm1[0] == '~' ) { sm1 = sm1.substr( 1u ); }
                       if ( sm2[0] == '~' ) { sm2 = sm2.substr( 1u ); }

                       if ( sm0[0] == 'w' ) { wire_operands += sm0; }
                       if ( sm1[0] == 'w' ) { wire_operands += sm1; }
                       if ( sm2[0] == 'w' ) { wire_operands += sm2; }
                     }
                     else
                     {
                       std::cout << "[w] cannot parse " << m[2] << std::endl;
                     }

                     for ( auto op : wire_operands )
                     {
                       add_edge( id, boost::lexical_cast<unsigned>( op.substr( 1u ) ), wire_dependency_graph );
                     }

                     assert( wires.size() == id );
                     f.target = num_inputs + id;

                     wires.push_back( f );
                   } },
                 { std::regex( "^assign (.*) = (.*);" ), [&]( const std::smatch& m ) {
                     if ( boost::find( output_names, m[1] ) != output_names.end() )
                     {
                       outputs += std::make_pair( m[1], m[2] );
                     }
                   } }
               } );

  assert( wires.size() == boost::num_vertices( wire_dependency_graph ) );
  std::vector<unsigned> topsort( wires.size() );
  boost::topological_sort( wire_dependency_graph, topsort.begin() );

  for ( const auto& w : topsort )
  {
    mig_function f;
    const auto& wire = wires[w];
    const auto& ops  = wire.operands;

    switch ( wire.operation )
    {
    case mighty_operation_t::opcode::_and:
      f = mig_create_and( mig, make_function( functions[ops[0].first], ops[0].second ), make_function( functions[ops[1].first], ops[1].second ) );
      break;
    case mighty_operation_t::opcode::_or:
      f = mig_create_or( mig, make_function( functions[ops[0].first], ops[0].second ), make_function( functions[ops[1].first], ops[1].second ) );
      break;
    case mighty_operation_t::opcode::_maj:
      f = mig_create_maj( mig,
                          make_function( functions[ops[0].first], ops[0].second ),
                          make_function( functions[ops[1].first], ops[1].second ),
                          make_function( functions[ops[2].first], ops[2].second ) );
      break;
    }

    functions[wire.target] = f;
  }

  for ( const auto& o : outputs )
  {
    mig_function f;

    if ( o.second == "one" )
    {
      f = mig_get_constant( mig, true );
    }
    else if ( o.second == "~one" )
    {
      f = mig_get_constant( mig, false );
    }
    else
    {
      const auto op = get_operand( o.second );
      f = make_function( functions[op.first], op.second );
    }
    mig_create_po( mig, f, o.first );
  }

  return mig;
}

void write_verilog( const mig_graph& mig, std::ostream& os,
                    const properties::ptr& settings,
                    const properties::ptr& statistics )
{
  /* settings */
  const auto default_model_name = get( settings, "default_model_name", std::string( "top" ) );
  const auto write_header       = get( settings, "write_header",       true );
  const auto header_prefix      = get( settings, "header_prefix",      std::string( "written by CirKit" ) );

  const auto& info = mig_info( mig );

  /* model name */
  auto model_name = info.model_name;
  if ( model_name.empty() ) { model_name = default_model_name; }

  std::vector<std::string> inames, onames, wnames;

  /* input names */
  for ( const auto& input : info.inputs )
  {
    inames += info.node_names.at( input );
  }

  /* output names */
  for ( const auto& output : info.outputs )
  {
    onames += output.second;
  }

  /* wires */
  std::vector<unsigned> node_to_wire( num_vertices( mig ) );
  auto count = 0u;
  for ( const auto& node : boost::make_iterator_range( boost::vertices( mig ) ) )
  {
    if ( !boost::out_degree( node, mig ) ) { continue; }

    node_to_wire[node] = count;
    wnames += boost::str( format( "w%d" ) % count++ );
  }

  if ( write_header )
  {
    auto time   = std::chrono::system_clock::now();
    auto time_c = std::chrono::system_clock::to_time_t( time );
    os << "// " << header_prefix << " " << std::ctime( &time_c ) << std::endl;
  }

  os << format( "module %s (" ) % model_name << std::endl
     << "        " << any_join( inames, ", " ) << ", " << std::endl
     << "        " << any_join( onames, ", " ) << ");" << std::endl;

  // TODO constant
  os << "input " << any_join( inames, ", " ) << ";" << std::endl
     << "output " << any_join( onames, ", " ) << ";" << std::endl
     << "wire one, " << any_join( wnames, ", " ) << ";" << std::endl;

  /* compute gates */
  for ( const auto& node : boost::make_iterator_range( boost::vertices( mig ) ) )
  {
    if ( !boost::out_degree( node, mig ) ) { continue; }

    const auto children = get_children( mig, node );

    assert( children[0u].node <= children[1u].node && children[1u].node <= children[2u].node );

    /* binary gate? */
    if ( children[0u].node == 0u )
    {
      const auto c1 = boost::out_degree( children[1u].node, mig ) ? boost::str( format( "w%d" ) % node_to_wire[children[1u].node] ) : info.node_names.at( children[1u].node );
      const auto c2 = boost::out_degree( children[2u].node, mig ) ? boost::str( format( "w%d" ) % node_to_wire[children[2u].node] ) : info.node_names.at( children[2u].node );

      /* OR gate */
      if ( children[0u].complemented )
      {
        os << format( "assign w%d = %s%s | %s%s;" ) % node_to_wire[node] % ( children[1u].complemented ? "~" : "" ) % c1
                                                                         % ( children[2u].complemented ? "~" : "" ) % c2 << std::endl;
      }
      /* AND gate */
      else
      {
        os << format( "assign w%d = %s%s & %s%s;" ) % node_to_wire[node] % ( children[1u].complemented ? "~" : "" ) % c1
                                                                         % ( children[2u].complemented ? "~" : "" ) % c2 << std::endl;
      }
    }
    else
    {
      const auto c0 = boost::out_degree( children[0u].node, mig ) ? boost::str( format( "w%d" ) % node_to_wire[children[0u].node] ) : info.node_names.at( children[0u].node );
      const auto c1 = boost::out_degree( children[1u].node, mig ) ? boost::str( format( "w%d" ) % node_to_wire[children[1u].node] ) : info.node_names.at( children[1u].node );
      const auto c2 = boost::out_degree( children[2u].node, mig ) ? boost::str( format( "w%d" ) % node_to_wire[children[2u].node] ) : info.node_names.at( children[2u].node );

      os << format( "assign w%1% = (%2%%3% & %4%%5%) | (%2%%3% & %6%%7%) | (%4%%5% & %6%%7%);" ) % node_to_wire[node] % ( children[0u].complemented ? "~" : "" ) % c0
                                                                                                                      % ( children[1u].complemented ? "~" : "" ) % c1
                                                                                                                      % ( children[2u].complemented ? "~" : "" ) % c2 << std::endl;
    }
  }

  os << "assign one = 1;" << std::endl;
  for ( const auto& output : info.outputs )
  {
    if ( output.first.node == 0u )
    {
      os << format( "assign %s = %d;" ) % output.second % ( output.first.complemented ? "one" : "~one" ) << std::endl;
    }
    else
    {
      const auto c = boost::out_degree( output.first.node, mig ) ? boost::str( format( "w%d" ) % node_to_wire[output.first.node] ) : info.node_names.at( output.first.node );
      os << format( "assign %s = %s%s;" ) % output.second % ( output.first.complemented ? "~" : "" ) % c << std::endl;
    }
  }

  os << "endmodule" << std::endl;
}

void write_verilog( const mig_graph& mig, const std::string& filename,
                    const properties::ptr& settings,
                    const properties::ptr& statistics )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  write_verilog( mig, os, settings, statistics );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
