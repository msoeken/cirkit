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

/**
 * @author Heinz Riener
 */

#include <classical/aig.hpp>
#include <classical/utils/aig_utils.hpp>
#include <boost/format.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <regex>

namespace cirkit
{

struct names_map
{
  std::string operator()( const unsigned id )
  {
    const std::string name = (boost::format("var_%d") % id).str();
    auto it = name_map.find( id );
    if ( it == name_map.end() )
    {
      return name;
    }
    return it->second;
  }

  void insert( const unsigned id, const std::string& s )
  {
    std::string na;
    unsigned idx;
    if ( split_name( na, idx, s ) )
    {
      const std::string&& format_string = (boost::format("%s_%s") % na % idx).str();
      // std::cerr << "{ " << id << ',' << format_string << " }" << std::endl;
      name_map.insert( {id,format_string} );
      update_bv_map( na, idx );
    }
    else
    {
      // std::cerr << "{ " << id << ',' << s << " }" << std::endl;
      name_map.insert( {id,s} );
    }
  }

  void update_bv_map( const std::string& name, const unsigned idx )
  {
    auto it = bv_map.find( name );
    if ( it == bv_map.end() )
    {
      bv_map.insert( {name,idx} );
    }
    else
    {
      if ( idx > it->second )
      {
        bv_map[ name ] = idx;
      }
    }
  }

  std::map< unsigned, std::string > name_map;
  std::map< std::string, unsigned > bv_map;
};

void write_smt2( const aig_graph& aig, std::ostream& os, const bool word_level_vars = false )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );

  const auto& graph_info = boost::get_property( aig, boost::graph_name );
  const auto& indexmap = get( boost::vertex_name, aig );
  const auto& complementmap = boost::get( boost::edge_complement, aig );

  names_map names;

  /* no latches! */
  const unsigned num_latches = graph_info.cis.size();
  assert( num_latches == 0 && "Sequential circuits are yet not supported" );

  /* inputs */
  os << ";; *****   inputs   *****" << std::endl;
  unsigned index = 0u;
  for ( const auto& input : graph_info.inputs )
  {
    // std::cerr << "i" << input << '\n';
    auto it = graph_info.node_names.find( input );
    if ( it != graph_info.node_names.end() )
    {
      names.insert( input, it->second );
    }
    else
    {
      names.insert( input, (boost::format("input%d") % index).str() );
    }
    os << "(declare-fun " << names(input) << "() Bool)" << std::endl;
    ++index;
  }

  if ( word_level_vars )
  {
    for ( auto it : names.bv_map )
    {
      const unsigned w = it.second + 1;
      os << "(declare-fun " << it.first << "() (_ BitVec " << w << "))" << std::endl;
      for ( unsigned i = 0u; i < w; ++i )
      {
        os << "(assert (= ((_ extract " << i << ' ' << i << ") " << it.first << ") "<< "(ite " << it.first << "_" << i << " (_ bv1 1) (_ bv0 1))))" << std::endl;
      }
    }
  }

  /* declare variables for and gates */
  os << ";; *****   and gates   *****" << std::endl;
  for ( const auto& node : boost::make_iterator_range( vertices( aig ) ) )
  {
    const auto _out_degree = out_degree( node, aig );
    if ( _out_degree )
    {
      assert( _out_degree == 2u );
      const unsigned id = indexmap[node]/2;
      os << "(declare-fun " << names(id) << "() Bool)" << std::endl;
    }
  }

  /* and gates */
  os << ";; *****   and gates   *****" << std::endl;
  for ( const auto& node : boost::make_iterator_range( vertices( aig ) ) )
  {
    const auto _out_degree = out_degree( node, aig );
    if ( _out_degree )
    {
      assert( _out_degree == 2u );
      const unsigned id = indexmap[node]/2;
      os << "(assert (= " << names(id) << " (and";
      for ( const auto& edge : boost::make_iterator_range( out_edges( node, aig ) ) )
      {
        if ( complementmap[edge] )
        {
          os << " (not " << names(target( edge, aig )) << ")";
        }
        else
        {
          os << " " << names(target( edge, aig ));
        }
      }
      os << ")))" << std::endl;
    }
  }

  /* outputs */
  os << ";; *****   outputs   *****" << std::endl;
  index = 0u;
  for ( const auto& output : graph_info.outputs )
  {
    // const std::string& name = output.second;
    const unsigned id = output.first.node;
    // std::cerr << "o" << id << '\n';
    os << "(declare-fun " << (boost::format("output%d") % index).str() << "() Bool)" << std::endl;
    os << "(assert (= " << (boost::format("output%d") % index).str() << ' ';
    if (output.first.complemented)
    {
      std::cout << "(not " << names(id) << ")";
    }
    else
    {
      std::cout << names(id);
    }
    std::cout << "))" << std::endl;
    ++index;
  }
}

void write_smt2( const aig_graph& aig, const std::string& filename, const bool word_level_vars = false )
{
  std::filebuf fb;
  fb.open( filename.c_str(), std::ios::out );
  std::ostream os( &fb );
  write_smt2( aig, os, word_level_vars );
  fb.close();
}

}

#include <core/utils/program_options.hpp>
#include <classical/aig.hpp>
#include <classical/io/read_aiger.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  std::string filename;

  program_options opts;
  opts.add_options()
    ( "filename",   value<std::string>( &filename ),   "AIG filename (in ASCII AIGER format)" )
    ( "word-level-vars,w",                             "Word-level variables" )
    ;
  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_set( "filename" ) )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  aig_graph aig;
  try
  {
    read_aiger( aig, filename );
  }
  catch (const char *msg)
  {
    std::cerr << msg << std::endl;
    return 2;
  }

  write_smt2( aig, std::cout, opts.is_set("word-level-vars") );

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
