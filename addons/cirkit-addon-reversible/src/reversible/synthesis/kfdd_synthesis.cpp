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
