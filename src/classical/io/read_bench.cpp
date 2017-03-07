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

#include "read_bench.hpp"

#include <core/utils/conversion_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/string_utils.hpp>
#include <classical/utils/aig_utils.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/algorithm.hpp>

#include <range/v3/algorithm/find.hpp>
#include <range/v3/algorithm/find_if.hpp>

#include <fstream>
#include <sstream>
#include <iomanip>
#include <iterator>

namespace cirkit
{

namespace detail
{

/**
 * Based on http://stackoverflow.com/questions/5607589/right-way-to-split-an-stdstring-into-a-vectorstring
 */
struct tokens : std::ctype< char >
{
  tokens() : std::ctype<char>( get_table() ) {}

  static std::ctype_base::mask const* get_table()
  {
    static constexpr const char SYM_COMMA  = ',';
    static constexpr const char SYM_ASSIGN = '=';
    static constexpr const char SYM_LPAREN = '(';
    static constexpr const char SYM_RPAREN = ')';
    static constexpr const char SYM_SPACE  = ' ';

    using cctype = std::ctype< char >;
    static const cctype::mask* const_rc = cctype::classic_table();

    static cctype::mask rc[ cctype::table_size ];
    std::memcpy( rc, const_rc, cctype::table_size * sizeof(cctype::mask) );

    rc[static_cast<int>( SYM_COMMA )]  = std::ctype_base::space;
    rc[static_cast<int>( SYM_ASSIGN )] = std::ctype_base::space;
    rc[static_cast<int>( SYM_LPAREN )] = std::ctype_base::space;
    rc[static_cast<int>( SYM_RPAREN )] = std::ctype_base::space;
    rc[static_cast<int>( SYM_SPACE )]  = std::ctype_base::space;
    return &rc[0];
  }
};

}

void read_bench( aig_graph& aig, std::ifstream& is )
{
  std::map< std::string, aig_function > the_map;
  std::list< std::string > pos;

  aig_initialize( aig );

  std::string line;
  while ( std::getline( is, line ) )
  {
    // std::cout << line << '\n';
    if ( line == "" || boost::starts_with( line, "#" ) ) continue;
    std::istringstream iis( line );
    iis.imbue( std::locale( std::locale(), new detail::tokens() ) );

    std::istream_iterator< std::string > iit( iis ), eos;
    while ( iit != eos )
    {
      const auto token = *iit;
      // std::cout << token << '\n';
      if ( token == "INPUT" )
      {
        ++iit;
        const auto name = *iit;
        the_map.insert( {name, aig_create_pi( aig, name )} );
        ++iit;
        continue;
      }

      if ( token == "OUTPUT" )
      {
        ++iit;
        const auto name = *iit;
        pos.push_back( name );
        ++iit;
        continue;
      }

      const std::string res = *iit; ++iit;
      const std::string kind = boost::to_upper_copy( *iit ); ++iit;
      std::vector< aig_function > ops;
      while ( iit != eos )
      {
        ops.push_back( the_map[ *iit ] );
        ++iit;
      }

      const auto num_ops = ops.size();
      assert( num_ops > 0u );

      /* unary operators */
      if ( num_ops == 1u )
      {
        assert( kind == "NOT" || kind == "BUF" );
        if ( kind == "NOT" )
        {
          the_map.insert( {res, !ops[0u]} );
        }
        else if ( kind == "BUF" )
        {
          the_map.insert( {res, aig_create_and( aig, ops[0u], ops[0u] ) } );
        }
        continue;
      }

      /* nary operators */
      if ( kind == "AND" )
      {
        the_map.insert( {res, aig_create_nary_and( aig, ops ) } );
        continue;
      }
      else if ( kind == "NAND" )
      {
        the_map.insert( {res, aig_create_nary_nand( aig, ops ) } );
        continue;
      }
      else if ( kind == "OR" )
      {
        the_map.insert( {res, aig_create_nary_or( aig, ops ) } );
        continue;
      }
      else if ( kind == "NOR" )
      {
        the_map.insert( {res, aig_create_nary_nor( aig, ops ) } );
        continue;
      }
      else if ( kind == "XOR" )
      {
        the_map.insert( {res, aig_create_nary_xor( aig, ops ) } );
        continue;
      }

      // std::cout << kind << '\n';
      assert( false && "Yet not implemented gate type" );
    }
  }

  for ( const auto po : pos )
  {
    aig_create_po( aig, the_map[ po ], po );
  }
}

void read_bench( aig_graph& aig, const std::string& filename )
{
  std::ifstream is( filename.c_str() );
  read_bench( aig, is );
  auto& info = aig_info( aig );
  info.model_name = boost::filesystem::path( filename ).stem().string();
  is.close();
}

void read_bench( lut_graph_t& lut, const std::string& filename )
{
  using gate_def_t = std::tuple<std::string, std::string, std::vector<std::string>>;

  std::vector<std::string> inputs, outputs;
  std::vector<gate_def_t> gates;

  line_parser( filename, {
      {std::regex( "^INPUT\\((.*)\\)$" ), [&inputs]( const std::smatch& m ) {
          inputs.push_back( std::string( m[1] ) );
        }},
      {std::regex( "^OUTPUT\\((.*)\\)$" ), [&outputs]( const std::smatch& m ) {
          outputs.push_back( std::string( m[1] ) );
        }},
      {std::regex( "^(.*) = LUT (.*) \\( (.*) \\)$" ), [&gates]( const std::smatch& m ) {
          unsigned value;
          std::stringstream converter( m[2] );
          converter >> std::hex >> value;

          std::vector<std::string> arguments;
          split_string( arguments, m[3], ", " );

          std::string name( m[1] );
          boost::trim( name );
          gates.push_back( std::make_tuple( name, std::string( m[2] ).substr( 2u ), arguments ) );

          //boost::dynamic_bitset<> direct( 1u << arguments.size(), value );
          //boost::dynamic_bitset<> indirect( convert_hex2bin( std::string( m[2] ).substr( 2u ) ) );

          //std::cout << "value: " << value << " orig: " << m[2] << " " << direct << " " << indirect << std::endl;
        }},
      {std::regex( "^(.*) = gnd" ), [&gates]( const std::smatch& m ) {
          std::string name( m[1] );
          boost::trim( name );

          gates.push_back( std::make_tuple( name, "gnd", std::vector<std::string>() ) );
        }},
      {std::regex( "^(.*) = vdd" ), [&gates]( const std::smatch& m ) {
          std::string name( m[1] );
          boost::trim( name );

          gates.push_back( std::make_tuple( name, "vdd", std::vector<std::string>() ) );
        }},
      {std::regex( "^#" ), []( const std::smatch& m ) {}}
    }, true );

  std::map<std::string, lut_vertex_t> gate_to_node;

  auto types = boost::get( boost::vertex_lut_type, lut );
  auto luts  = boost::get( boost::vertex_lut, lut );
  auto names = boost::get( boost::vertex_name, lut );

  auto v_gnd = add_vertex( lut );
  types[v_gnd] = lut_type_t::gnd;
  gate_to_node["gnd"] = v_gnd;
  names[v_gnd] = "gnd";
  auto v_vdd = add_vertex( lut );
  types[v_vdd] = lut_type_t::vdd;
  gate_to_node["vdd"] = v_vdd;
  names[v_vdd] = "vdd";

  for ( const auto& input : inputs )
  {
    auto v = add_vertex( lut );
    names[v] = input;
    types[v] = lut_type_t::pi;
    gate_to_node[input] = v;
  }

  /* gates may not be in topological order, so we need to keep track of this */
  std::vector<int>              waiting_refs( gates.size(), -1 ); /* if -1, gate has not been processed or is done, otherwise gives the number of fanins to wait for */
  std::vector<std::vector<int>> notify_list( gates.size() );      /* which parents to notify that they may complete */

  for ( const auto& it : index( gates ) )
  {
    const auto& gate = it.value;
    const auto& index = it.index;

    waiting_refs[index] = 0;

    if ( std::get<2>( gate ).empty() )
    {
      std::cout << "[i] assign " << std::get<0>( gate ) << " to constant" << std::endl;
      gate_to_node[std::get<0>( gate )] = std::get<1>( gate ) == "vdd" ? v_vdd : v_gnd;
    }
    else
    {
      for ( const auto& arg : std::get<2>( gate ) )
      {
        if ( gate_to_node.find( arg ) == gate_to_node.end() )
        {
          waiting_refs[index]++;
          notify_list[std::distance( begin( gates ), ranges::find_if( gates, [&arg]( const gate_def_t& g ) { return std::get<0>( g ) == arg; } ) )].push_back( index );
        }
      }
    }

    if ( waiting_refs[index] == 0 )
    {
      std::vector<unsigned> to_create;

      std::stack<unsigned> stack;
      stack.push( index );

      while ( !stack.empty() )
      {
        const auto n = stack.top();
        stack.pop();

        if ( ranges::find( to_create, n ) == end( to_create ) )
        {
          to_create.push_back( n );
        }

        for ( auto parent : notify_list[n] )
        {
          if ( --waiting_refs[parent] == 0 )
          {
            stack.push( parent );
          }
        }
      }

      for ( auto n : to_create )
      {
        const auto& gate = gates[n];

        if ( std::get<1>( gate ) == "vdd" || std::get<1>( gate ) == "gnd" )
        {
          continue;
        }

        auto v = add_vertex( lut );

        types[v] = lut_type_t::internal;
        luts[v] = std::get<1>( gate );

        for ( const auto& arg : std::get<2>( gate ) )
        {
          if ( gate_to_node.find( arg ) == gate_to_node.end() )
          {
            std::cout << "[e] cannot find gate " << arg << " when constructing LUT for " << std::get<0>( gate ) << std::endl;
            assert( false );
          }
          add_edge( v, gate_to_node[arg], lut );
        }

        gate_to_node[std::get<0>( gate )] = v;
      }
    }
  }

  for ( const auto& output : outputs )
  {
    auto v = add_vertex( lut );

    if ( gate_to_node.find( output ) == gate_to_node.end() )
    {
      std::cout << "[e] cannot find gate " << output << " when constructing output" << std::endl;
      assert( false );
    }

    add_edge( v, gate_to_node[output], lut );

    types[v] = lut_type_t::po;
    names[v] = output;
  }
}

void read_bench( lut_graph& graph, const std::string& filename )
{
  using gate_def_t = std::tuple<std::string, std::string, std::vector<std::string>>;

  std::vector<std::string> inputs, outputs;
  std::vector<gate_def_t> gates;

  line_parser( filename, {
      {std::regex( "^INPUT\\((.*)\\)$" ), [&inputs]( const std::smatch& m ) {
          inputs.push_back( std::string( m[1] ) );
        }},
      {std::regex( "^OUTPUT\\((.*)\\)$" ), [&outputs]( const std::smatch& m ) {
          outputs.push_back( std::string( m[1] ) );
        }},
      {std::regex( "^(.*) = LUT (.*) \\( (.*) \\)$" ), [&gates]( const std::smatch& m ) {
          unsigned value;
          std::stringstream converter( m[2] );
          converter >> std::hex >> value;

          std::vector<std::string> arguments;
          split_string( arguments, m[3], ", " );

          std::string name( m[1] );
          boost::trim( name );
          gates.push_back( std::make_tuple( name, std::string( m[2] ).substr( 2u ), arguments ) );

          //boost::dynamic_bitset<> direct( 1u << arguments.size(), value );
          //boost::dynamic_bitset<> indirect( convert_hex2bin( std::string( m[2] ).substr( 2u ) ) );

          //std::cout << "value: " << value << " orig: " << m[2] << " " << direct << " " << indirect << std::endl;
        }},
      {std::regex( "^(.*) = gnd" ), [&gates]( const std::smatch& m ) {
          std::string name( m[1] );
          boost::trim( name );

          gates.push_back( std::make_tuple( name, "gnd", std::vector<std::string>() ) );
        }},
      {std::regex( "^(.*) = vdd" ), [&gates]( const std::smatch& m ) {
          std::string name( m[1] );
          boost::trim( name );

          gates.push_back( std::make_tuple( name, "vdd", std::vector<std::string>() ) );
        }},
      {std::regex( "^#" ), []( const std::smatch& m ) {}}
    }, true );

  std::map<std::string, lut_vertex_t> gate_to_node;

  gate_to_node["gnd"] = graph.get_constant(false);
  gate_to_node["vdd"] = graph.get_constant(true);

  for ( const auto& input : inputs )
  {
    gate_to_node[input] = graph.create_pi( input );
  }

  /* gates may not be in topological order, so we need to keep track of this */
  std::vector<int>              waiting_refs( gates.size(), -1 ); /* if -1, gate has not been processed or is done, otherwise gives the number of fanins to wait for */
  std::vector<std::vector<int>> notify_list( gates.size() );      /* which parents to notify that they may complete */

  for ( const auto& it : index( gates ) )
  {
    const auto& gate = it.value;
    const auto& index = it.index;

    waiting_refs[index] = 0;

    if ( std::get<2>( gate ).empty() )
    {
      std::cout << "[i] assign " << std::get<0>( gate ) << " to constant" << std::endl;
      gate_to_node[std::get<0>( gate )] = graph.get_constant( std::get<1>( gate ) == "vdd" );
    }
    else
    {
      for ( const auto& arg : std::get<2>( gate ) )
      {
        if ( gate_to_node.find( arg ) == gate_to_node.end() )
        {
          waiting_refs[index]++;
          notify_list[std::distance( begin( gates ), ranges::find_if( gates, [&arg]( const gate_def_t& g ) { return std::get<0>( g ) == arg; } ) )].push_back( index );
        }
      }
    }

    if ( waiting_refs[index] == 0 )
    {
      std::vector<unsigned> to_create;

      std::stack<unsigned> stack;
      stack.push( index );

      while ( !stack.empty() )
      {
        const auto n = stack.top();
        stack.pop();

        if ( ranges::find( to_create, n ) == end( to_create ) )
        {
          to_create.push_back( n );
        }

        for ( auto parent : notify_list[n] )
        {
          if ( --waiting_refs[parent] == 0 )
          {
            stack.push( parent );
          }
        }
      }

      for ( auto n : to_create )
      {
        const auto& gate = gates[n];

        if ( std::get<1>( gate ) == "gnd" )
        {
          gate_to_node[std::get<0>( gate )] = graph.get_constant(false);
          continue;
        }
        else if ( std::get<1>( gate ) == "vdd" )
        {
          gate_to_node[std::get<0>( gate )] = graph.get_constant(true);
          continue;
        }

        std::vector<lut_vertex_t> ops;
        for ( const auto& arg : std::get<2>( gate ) )
        {
          if ( gate_to_node.find(arg) == std::end(gate_to_node) )
          {
            std::cout << "[e] cannot find gate " << arg << " when constructing LUT for " << std::get<0>( gate ) << std::endl;
            assert( false );
          }
          ops += gate_to_node[arg];
        }
        
        gate_to_node[std::get<0>( gate )] = graph.create_lut( std::get<1>( gate ), ops, std::get<0>( gate ) );
      }
    }
  }

  for ( auto i = 0u; i < outputs.size(); ++i )
  {
    const auto name = outputs[i];
    if ( gate_to_node.find(outputs[i]) == std::end(gate_to_node) )
    {
      std::cout << "[e] cannot find gate " << outputs[i] << " when constructing output" << std::endl;
      assert( false );
    }
    else
    {
      graph.create_po( gate_to_node[outputs[i]], name );
    }
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
