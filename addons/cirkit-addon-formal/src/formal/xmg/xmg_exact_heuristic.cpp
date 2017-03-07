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

#include "xmg_exact_heuristic.hpp"

#include <iostream>

#include <core/utils/timer.hpp>
#include <classical/functions/aig_from_truth_table.hpp>
#include <classical/xmg/xmg_aig.hpp>
#include <formal/synthesis/exact_mig.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

xmg_graph xmg_exact_heuristic( const tt& spec, const properties::ptr& settings, const properties::ptr& statistics )
{
  /* settings */
  const auto timeout = get( settings, "timeout", 60u );
  const auto verbose = get( settings, "verbose", false );

  /* statistics */
  properties_timer t( statistics );

  /* base line */
  const auto aig = aig_from_truth_table( spec );
  auto xmg = xmg_from_aig( aig );
  unsigned start = xmg.size();

  if ( verbose )
  {
    std::cout << "[i] upper bound from AIG: " << start << std::endl;
  }

  /* already best? */
  if ( start <= 1u )
  {
    set( statistics, "guaranteed_optimum", false );
    return xmg;
  }

  /* try to improve */
  auto es_settings = std::make_shared<properties>();
  es_settings->set( "verbose", verbose );
  es_settings->set( "timeout", boost::optional<unsigned>( timeout ) );
  while ( true )
  {
    es_settings->set( "start", start - 1 );
    auto res = exact_xmg_with_sat( spec, es_settings );

    if ( (bool)res )
    {
      const auto xmg_new = *res;
      if ( xmg_new.size() == start )
      {
        /* no more improvement */
        set( statistics, "guaranteed_optimum", true );
        break;
      }
      else
      {
        assert( xmg_new.size() == start - 1 );
        xmg = xmg_new;
        --start;
      }
    }
    else
    {
      /* stop */
      set( statistics, "guaranteed_optimum", false );
      break;
    }
  }

  return xmg;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
