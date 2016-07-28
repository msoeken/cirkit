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
  xmg_dot_writer( xmg_graph& xmg )
    : xmg( xmg ) {}

  void operator()( std::ostream& os, xmg_node n )
  {
    string_properties_map_t properties;

    if ( xmg.is_input( n ) )
    {
      properties["style"] = "filled";
      properties["fillcolor"] = "snow2";
      properties["shape"] = "house";
    }
    else if ( xmg.is_xor( n ) )
    {
      properties["style"] = "filled";
      properties["fillcolor"] = "lightskyblue";
      properties["label"] = "<<font point-size=\"11\">XOR</font>";

      properties["label"] += boost::str( boost::format( "<br/><font point-size=\"10\">%d</font>" ) % n );
      properties["label"] += ">";
    }
    else if ( xmg.is_maj( n ) )
    {
      properties["style"] = "filled";

      if ( *( boost::adjacent_vertices( n, xmg.graph() ).first ) != 0 )
      {
        properties["fillcolor"] = "lightsalmon";
        properties["label"] = "<<font point-size=\"11\">MAJ</font>";
      }
      else
      {
        if ( xmg.complement()[*( boost::out_edges( n, xmg.graph() ).first )] )
        {
          properties["fillcolor"] = "palegreen2";
          properties["label"] = "<<font point-size=\"11\">OR</font>";
        }
        else
        {
          properties["fillcolor"] = "lightcoral";
          properties["label"] = "<<font point-size=\"11\">AND</font>";
        }
      }

      properties["label"] += boost::str( boost::format( "<br/><font point-size=\"10\">%d</font>" ) % n );
      properties["label"] += ">";
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
    if ( boost::target( e, xmg.graph() ) == 0 )
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
      os << "o" << index << "[label=\"" << o.second << "\",shape=house,fillcolor=snow2,style=filled];" << std::endl;
      os << "o" << index << " -> " << o.first.node << " ";
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
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void write_dot( xmg_graph& xmg, std::ostream& os,
                const properties::ptr& settings, const properties::ptr& statistics )
{
  xmg_dot_writer writer( xmg );
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
