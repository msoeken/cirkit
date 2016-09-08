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
