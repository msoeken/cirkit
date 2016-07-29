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

#include "xmg_show.hpp"

#include <fstream>
#include <iostream>

#include <boost/graph/graphviz.hpp>

#include <core/utils/range_utils.hpp>
#include <core/utils/string_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

struct xmg_dot_writer
{
public:
  xmg_dot_writer( xmg_graph& xmg, const properties::ptr& settings )
    : xmg( xmg )
  {
    xor_color = get( settings, "xor_color", xor_color );
    maj_color = get( settings, "maj_color", maj_color );
    and_color = get( settings, "and_color", and_color );
    or_color  = get( settings, "or_color",  or_color );
    io_color  = get( settings, "io_color",  io_color );

    show_and_or_edges = get( settings, "show_and_or_edges", show_and_or_edges );
    show_node_ids     = get( settings, "show_node_ids",     show_node_ids );
  }

  std::string get_node_label( xmg_node n, const std::string& label )
  {
    std::string str = boost::str( boost::format( "<<font point-size=\"11\">%s</font>" ) % label );
    if ( show_node_ids )
    {
      str += boost::str( boost::format( "<br/><font point-size=\"10\">%d</font>" ) % n );
    }
    str += ">";

    return str;
  }

  void operator()( std::ostream& os, xmg_node n )
  {
    string_properties_map_t properties;

    if ( xmg.is_input( n ) )
    {
      properties["style"] = "filled";
      properties["fillcolor"] = io_color;
      properties["shape"] = "house";
      properties["label"] = get_node_label( n, n == 0 ? "0" : xmg.input_name( n ) );
    }
    else if ( xmg.is_xor( n ) )
    {
      properties["style"] = "filled";
      properties["fillcolor"] = xor_color;
      properties["label"] = get_node_label( n, "XOR" );
    }
    else if ( xmg.is_maj( n ) )
    {
      properties["style"] = "filled";

      if ( *( boost::adjacent_vertices( n, xmg.graph() ).first ) != 0 )
      {
        properties["fillcolor"] = maj_color;
        properties["label"] = get_node_label( n, "MAJ" );
      }
      else
      {
        if ( xmg.complement()[*( boost::out_edges( n, xmg.graph() ).first )] )
        {
          properties["fillcolor"] = or_color;
          properties["label"] = get_node_label( n, "OR" );
        }
        else
        {
          properties["fillcolor"] = and_color;
          properties["label"] = get_node_label( n, "AND" );
        }
      }
    }

    if ( xmg.is_marked( n ) )
    {
      properties["fillcolor"] = "red";
      properties["style"] = "filled";
    }

    os << "[" << make_properties_string( properties ) << "]";
  }

  void operator()( std::ostream& os, const xmg_edge& e )
  {
    if ( !show_and_or_edges && boost::target( e, xmg.graph() ) == 0 )
    {
      os << "[style=invis]";
    }
    else if ( xmg.complement()[e] )
    {
      os << "[style=dashed]";
    }
  }

  void operator()( std::ostream& os )
  {
    /* outputs */
    auto index = 0u;
    for ( const auto& o : xmg.outputs() )
    {
      os << boost::format( "o%d[label=<<font point-size=\"11\">%s</font>>,shape=house,fillcolor=%s,style=filled];" ) % index % o.second % io_color << std::endl;
      os << boost::format( "o%d -> %d " ) % index % o.first.node;
      if ( o.first.complemented )
      {
        os << "[style=dashed]";
      }
      os << ";" << std::endl;
      ++index;
    }

    /* levels */
    xmg.compute_levels();
    auto max_level = 0u;
    for ( const auto& o : xmg.outputs() )
    {
      max_level = std::max( xmg.level( o.first.node ), max_level );
    }

    std::vector<std::vector<xmg_node>> ranks( max_level + 2u );

    for ( const auto& n : xmg.nodes() )
    {
      ranks[xmg.level(n)].push_back( n );
    }

    for ( const auto& r : ranks )
    {
      if ( r.empty() ) { continue; }
      os << "{rank = same; " << any_join( r, "; " ) << ";}" << std::endl;
    }

    os << "{rank = same;";
    for ( auto i = 0; i < xmg.outputs().size(); ++i )
    {
      os << " o" << i << ";";
    }
    os << "}" << std::endl;
  }

private:
  xmg_graph& xmg;

  std::string xor_color = "lightskyblue";
  std::string maj_color = "lightsalmon";
  std::string and_color = "lightcoral";
  std::string or_color  = "palegreen2";
  std::string io_color  = "snow2";

  bool show_and_or_edges = false;
  bool show_node_ids = false;
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void write_dot( xmg_graph& xmg, std::ostream& os,
                const properties::ptr& settings, const properties::ptr& statistics )
{
  xmg_dot_writer writer( xmg, settings );
  write_graphviz( os, xmg.graph(), writer, writer, writer );
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void write_dot( xmg_graph& xmg, const std::string& filename,
                const properties::ptr& settings, const properties::ptr& statistics )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  write_dot( xmg, os, settings, statistics );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
