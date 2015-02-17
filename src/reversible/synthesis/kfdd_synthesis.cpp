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

#include "kfdd_synthesis.hpp"

#include <core/utils/timer.hpp>

#include "dd_synthesis_p.hpp"

namespace cirkit
{

  bool kfdd_synthesis( circuit& circ, const std::string& filename, properties::ptr settings, properties::ptr statistics )
  {
    using namespace internal;

    unsigned    default_decomposition = get<unsigned>( settings, "default_decomposition", kfdd_synthesis_dtl_shannon );
    unsigned    reordering            = get<unsigned>( settings, "reordering", kfdd_synthesis_reordering_none );
    double      sift_factor           = get<double>( settings, "sift_factor", 2.5 );
    char        sifting_growth_limit  = get<char>( settings, "sifting_growth_limit", kfdd_synthesis_growth_limit_absolute );
    char        sifting_method        = get<char>( settings, "sifting_method", kfdd_synthesis_sifting_method_verify );
    std::string dotfilename           = get<std::string>( settings, "dotfilename", std::string() );

    // run-time measurement
    properties_timer t( statistics );

    dd_from_kfdd_settings _settings;
    _settings.default_decomposition = default_decomposition;
    _settings.reordering            = reordering;
    _settings.sift_factor           = sift_factor;
    _settings.sifting_growth_limit  = sifting_growth_limit;
    _settings.sifting_method        = sifting_method;

    unsigned node_count;
    if ( statistics )
    {
      _settings.node_count = &node_count;
    }

    dd graph;
    dd_from_kfdd( graph, filename, _settings );

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

  pla_blif_synthesis_func kfdd_synthesis_func( properties::ptr settings, properties::ptr statistics )
  {
    pla_blif_synthesis_func f = [&settings, &statistics]( circuit& circ, const std::string& filename ) {
      return kfdd_synthesis( circ, filename, settings, statistics );
    };
    f.init( settings, statistics );
    return f;
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
