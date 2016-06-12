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

#include "mig_mighty_rewriting.hpp"

#include <core/utils/timer.hpp>


#ifdef ADDON_EPFL
#include <classical/mig/mig_mighty.hpp>
#endif

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

mig_graph mig_mighty_depth_rewriting( const mig_graph& mig, const properties::ptr& settings, const properties::ptr& statistics )
{
  properties_timer t( statistics );

#ifdef ADDON_EPFL

  auto * _mig = mig_to_mighty( mig );
  computedepth( _mig );
  selective_depth( _mig );
  aggressive_depth( _mig, 1.2 );
  selective_depth( _mig );
  const auto new_mig = mig_from_mighty( _mig );
  freemig( _mig );
  return new_mig;

#else

  return mig;

#endif
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
