/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

#include "xmg_io.hpp"

#include <fstream>
#include <unordered_map>

#include <boost/algorithm/string/join.hpp>
#include <boost/format.hpp>

#include <core/utils/string_utils.hpp>

using boost::format;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

std::string escape_name( const std::string& name )
{
  if ( name.find( "[" ) )
  {
    return "\\" + name;
  }
  else
  {
    return name;
  }
}

std::string unescape_name( const std::string& name )
{
  if ( !name.empty() && name[0] == '\\' )
  {
    return name.substr( 1u );
  }
  else
  {
    return name;
  }
}

std::vector<std::string> get_input_names( const xmg_graph& xmg )
{
  std::vector<std::string> inames;
  for ( const auto& input : xmg.inputs() )
  {
    inames.push_back( escape_name( input.second ) );
  }
  return inames;
}

std::vector<std::string> get_output_names( const xmg_graph& xmg )
{
  std::vector<std::string> onames;
  for ( const auto& output : xmg.outputs() )
  {
    onames.push_back( escape_name( output.second ) );
  }
  return onames;
}

std::vector<std::string> get_wire_names( const xmg_graph& xmg )
{
  std::vector<std::string> wnames;
  for ( auto v : xmg.nodes() )
  {
    if ( xmg.is_input( v ) ) { continue; }
    wnames.push_back( boost::str( format( "w%d" ) % v ) );
  }
  return wnames;
}

std::string get_operand_name( const xmg_graph& xmg, const xmg_function& f )
{
  std::string name;

  if ( f.node == 0u )
  {
    name = "zero";
  }
  else if ( xmg.is_input( f.node ) )
  {
    name = escape_name( xmg.input_name( f.node ) );
  }
  else
  {
    name = boost::str( format( "w%d" ) % f.node );
  }

  if ( f.complemented )
  {
    name = "~" + name;
  }

  return name;
}

void write_bench( const xmg_graph& xmg, std::ostream& os )
{
  // TODO
  assert( false && "not implemented yet" );

  for ( const auto& input : xmg.inputs() )
  {
    os << format( "INPUT(%s)" ) % input.second << std::endl;
  }
  for ( const auto& output : xmg.outputs() )
  {
    os << format( "OUTPUT(%s)" ) % output.second << std::endl;
  }

  for ( auto v : xmg.topological_nodes() )
  {
    if ( xmg.is_xor( v ) )
    {
    }
    else if ( xmg.is_maj( v ) )
    {
    }
  }
}

void write_verilog( const xmg_graph& xmg, std::ostream& os, const properties::ptr& settings )
{
  const auto default_name = get( settings, "default_name", std::string( "top" ) );
  const auto maj_module   = get( settings, "maj_module",   false );

  /* MAJ module */
  if ( maj_module )
  {
    os << "module CKT_MAJ(a, b, c, f);" << std::endl
       << "  input a, b, c;" << std::endl
       << "  output f;" << std::endl
       << "  assign f = (a & b) | (a & c) | (b & c);" << std::endl
       << "endmodule" << std::endl << std::endl;
  }

  /* top module */
  auto name = xmg.name().empty() ? default_name : xmg.name();

  auto inames = get_input_names( xmg );
  auto onames = get_output_names( xmg );
  auto wnames = get_wire_names( xmg );

  wnames.insert( wnames.begin(), "zero" );

  os << format( "module %s( %s , %s );" ) % name % boost::join( inames, " , " ) % boost::join( onames, " , " ) << std::endl;

  os << format( "  input %s ;" ) % boost::join( inames, " , " ) << std::endl
     << format( "  output %s ;" ) % boost::join( onames, " , " ) << std::endl
     << format( "  wire %s ;" ) % boost::join( wnames, " , " ) << std::endl;

  os << "  assign zero = 0;" << std::endl;

  for ( auto v : xmg.topological_nodes() )
  {
    if ( xmg.is_input( v ) ) { continue; }

    const auto c = xmg.children( v );

    if ( xmg.is_xor( v ) )
    {
      os << format( "  assign w%d = %s ^ %s ;" ) % v % get_operand_name( xmg, c[0] ) % get_operand_name( xmg, c[1] ) << std::endl;
    }
    else if ( xmg.is_maj( v ) )
    {
      if ( c[0].node == 0u && !c[0].complemented ) /* AND */
      {
        os << format( "  assign w%d = %s & %s ;" ) % v % get_operand_name( xmg, c[1] ) % get_operand_name( xmg, c[2] ) << std::endl;
      }
      else if ( c[0].node == 0u && c[0].complemented ) /* OR */
      {
        os << format( "  assign w%d = %s | %s ;" ) % v % get_operand_name( xmg, c[1] ) % get_operand_name( xmg, c[2] ) << std::endl;
      }
      else
      {
        if ( maj_module )
        {
          os << format( "  CKT_MAJ maj%4%( %1% , %2% , %3% , w%4% );" ) % get_operand_name( xmg, c[0] ) % get_operand_name( xmg, c[1] ) % get_operand_name( xmg, c[2] ) % v << std::endl;
        }
        else
        {
          os << format( "  assign w%4% = ( %1% & %2% ) | ( %1% & %3% ) | ( %2% & %3% ) ;" ) % get_operand_name( xmg, c[0] ) % get_operand_name( xmg, c[1] ) % get_operand_name( xmg, c[2] ) % v << std::endl;
        }
      }
    }
  }

  for ( const auto& output : xmg.outputs() )
  {
    os << format( "  assign %s = %s ;" ) % escape_name( output.second ) % get_operand_name( xmg, output.first ) << std::endl;
  }

  os << "endmodule" << std::endl;
}

xmg_function find_function( const std::unordered_map<std::string, xmg_function>& name_to_function, const std::string& name )
{
  if ( name[0] == '~' )
  {
    return !name_to_function.at( name.substr( 1u ) );
  }
  else
  {
    return name_to_function.at( name );
  }
}

std::string make_regular( const std::string& name )
{
  if ( !name.empty() && name[0] == '~' )
  {
    return name.substr( 1u );
  }
  else
  {
    return name;
  }
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

xmg_graph read_verilog( const std::string& filename, bool native_xor, bool enable_structural_hashing, bool enable_inverter_propagation )
{
  std::unordered_map<std::string, xmg_function> name_to_function;
  std::vector<std::string> output_names;

  xmg_graph xmg;
  xmg.set_native_xor( native_xor );
  xmg.set_structural_hashing( enable_structural_hashing );
  xmg.set_inverter_propagation( enable_inverter_propagation );

  name_to_function.insert( {"1'b0", xmg.get_constant( false )} );
  name_to_function.insert( {"1'b1", xmg.get_constant( true )} );

  /* to store the instructions */
  struct inst_t
  {
    enum op_t { OP_AND, OP_OR, OP_XOR, OP_MAJ, OP_CONST, OP_BUF };

    std::string target;
    std::array<std::string, 3> operands;
    op_t opcode;
  };
  digraph_t<inst_t> instructions;
  std::unordered_map<std::string, unsigned> name_to_vertex;

  line_parser( filename, {
      {std::regex( "^module +(.*) +\\(" ), [&xmg]( const std::smatch& m ) {
          xmg.set_name( std::string( m[1] ) );
        }},
      {std::regex( "input (.*);" ), [&xmg, &name_to_function]( const std::smatch& m ) {
          foreach_string( m[1], ", ", [&xmg, &name_to_function]( const std::string& name ) {
              name_to_function.insert( {name, xmg.create_pi( unescape_name( name ) )} );
            } );
        }},
      {std::regex( "output (.*);" ), [&output_names]( const std::smatch& m ) {
          split_string( output_names, m[1], ", " );
        }},
      {std::regex( "assign (.*) = (.*);" ), [&output_names, &instructions, &name_to_vertex]( const std::smatch& m ) {
          auto name = std::string( m[1] );
          auto expr = std::string( m[2] );

          boost::trim( name );
          boost::trim( expr );

          std::smatch match;

          const auto add_instruction = [&instructions, &name_to_vertex]( const std::string& name, const inst_t& inst ) {
            const auto v = add_vertex( instructions );
            instructions[v] = inst;
            name_to_vertex.insert( {name, v} );
          };

          if ( std::regex_search( expr, match, std::regex( "^( *[^ ]+ *) & ( *[^ ]+ *)$" ) ) )
          {
            add_instruction( name, {name, {{std::string( match[1] ), std::string( match[2] )}}, inst_t::OP_AND} );
          }
          else if ( std::regex_search( expr, match, std::regex( "^( *[^ ]+ *) \\| ( *[^ ]+ *)$" ) ) )
          {
            add_instruction( name, {name, {{std::string( match[1] ), std::string( match[2] )}}, inst_t::OP_OR} );
          }
          else if ( std::regex_search( expr, match, std::regex( "^( *[^ ]+ *) \\^ ( *[^ ]+ *)$" ) ) )
          {
            add_instruction( name, {name, {{std::string( match[1] ), std::string( match[2] )}}, inst_t::OP_XOR} );
          }
          else if ( std::regex_search( expr, match, std::regex( "^\\( *([^ ]+) & ([^ ]+) *\\) \\| \\( *([^ ]+) & ([^ ]+) *\\) \\| \\( *([^ ]+) & ([^ ]+) *\\)$" ) ) )
          {
            add_instruction( name, {name, {{std::string( match[1] ), std::string( match[2] ), std::string( match[4] )}}, inst_t::OP_MAJ} );
          }
          else if ( expr == "0" )
          {
            add_instruction( name, {name, {{std::string( "0" )}}, inst_t::OP_CONST} );
          }
          else if ( expr == "1" )
          {
            add_instruction( name, {name, {{std::string( "1" )}}, inst_t::OP_CONST} );
          }
          else
          {
            if ( boost::find( output_names, name ) == output_names.end() )
            {
              std::cout << "[e] expected " << name << " not be part of output_names" << std::endl;
              std::cout << "[e] in line: " << m[0] << std::endl;
              assert( false );
            }
            add_instruction( name, {name, {{expr}}, inst_t::OP_BUF} );
          }
        }}}, false );

  for ( const auto v : boost::make_iterator_range( vertices( instructions ) ) )
  {
    const auto& inst = instructions[v];
    const auto s = name_to_vertex[inst.target];
    for ( auto t : inst.operands )
    {
      const auto it = name_to_vertex.find( make_regular( t ) );
      if ( it != name_to_vertex.end() )
      {
        add_edge( s, it->second, instructions );
      }
    }
  }

  std::vector<unsigned> top( num_vertices( instructions ) );
  boost::topological_sort( instructions, top.begin() );

  for ( const auto v : top )
  {
    const auto& inst = instructions[v];
    switch ( inst.opcode )
    {
    case inst_t::OP_AND:
      name_to_function.insert( {inst.target, xmg.create_and( find_function( name_to_function, inst.operands[0] ), find_function( name_to_function, inst.operands[1] ) )} );
      break;
    case inst_t::OP_OR:
      name_to_function.insert( {inst.target, xmg.create_or( find_function( name_to_function, inst.operands[0] ), find_function( name_to_function, inst.operands[1] ) )} );
      break;
    case inst_t::OP_XOR:
      name_to_function.insert( {inst.target, xmg.create_xor( find_function( name_to_function, inst.operands[0] ), find_function( name_to_function, inst.operands[1] ) )} );
      break;
    case inst_t::OP_MAJ:
      name_to_function.insert( {inst.target, xmg.create_maj( find_function( name_to_function, inst.operands[0] ), find_function( name_to_function, inst.operands[1] ), find_function( name_to_function, inst.operands[2] ) )} );
      break;
    case inst_t::OP_CONST:
      name_to_function.insert( {inst.target, xmg.get_constant( inst.operands[0] == "0" ? false : true )} );
      break;
    case inst_t::OP_BUF:
      name_to_function.insert( {inst.target, find_function( name_to_function, inst.operands[0] )} );
      break;
    }
  }

  for ( const auto& name : output_names )
  {
    xmg.create_po( name_to_function[name], unescape_name( name ) );
  }

  return xmg;
}

void write_bench( const xmg_graph& xmg, const std::string& filename )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  write_bench( xmg, os );
}

void write_verilog( const xmg_graph& xmg, const std::string& filename, const properties::ptr& settings )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  write_verilog( xmg, os, settings );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
