/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include "bdd_synthesis.hpp"

#include <cudd.h>

#include <core/utils/timer.hpp>

#include "dd_synthesis_p.hpp"

namespace revkit
{

  bool bdd_synthesis( circuit& circ, const std::string& filename,
                      properties::ptr settings,
                      properties::ptr statistics )
  {
    using namespace internal;

    bool        complemented_edges  = get<bool>( settings, "complemented_edges", true );
    unsigned    reordering          = get<unsigned>( settings, "reordering", CUDD_REORDER_SIFT );
    std::string dotfilename         = get<std::string>( settings, "dotfilename", std::string() );
    std::string infofilename        = get<std::string>( settings, "infofilename", std::string() );

    // run-time measurement
    timer<properties_timer> t;

    if ( statistics )
    {
      properties_timer rt( statistics );
      t.start( rt );
    }

    dd_from_bdd_settings _settings;
    _settings.complemented_edges = complemented_edges;
    _settings.reordering = reordering;
    _settings.infofilename = infofilename;

    unsigned node_count = 0u;
    if ( statistics )
    {
      _settings.node_count = &node_count;
    }

    dd graph;
    dd_from_bdd( graph, filename, _settings );

    if ( dotfilename.size() )
    {
      dd_to_dot( graph, dotfilename );
    }

    if ( statistics )
    {
      statistics->set( "node_count", node_count );
    }

    dd_synthesis( circ, graph );

    return true;
  }

  pla_blif_synthesis_func bdd_synthesis_func( properties::ptr settings, properties::ptr statistics )
  {
    pla_blif_synthesis_func f = [&settings, &statistics]( circuit& circ, const std::string& filename ) {
      return bdd_synthesis( circ, filename, settings, statistics );
    };
    f.init( settings, statistics );
    return f;
  }

}

// Local Variables:
// c-basic-offset: 2
// End:
