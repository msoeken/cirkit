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

#include "bdd_level_approximation.hpp"

#include <functional>

#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

#include <core/utils/timer.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

bdd approximate( const bdd& f, bdd_level_approximation_mode mode, unsigned level )
{
  switch ( mode )
  {
  case bdd_level_approximation_mode::round_down:
    return f.round_down( level );
  case bdd_level_approximation_mode::round_up:
    return f.round_up( level );
  case bdd_level_approximation_mode::round:
    return f.round( level );
  case bdd_level_approximation_mode::cof0:
    return f.cof0( level );
  case bdd_level_approximation_mode::cof1:
    return f.cof1( level );
  }

  assert( false );
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

std::vector<bdd> bdd_level_approximation( const std::vector<bdd>& fs, bdd_level_approximation_mode mode, unsigned level,
                                          const properties::ptr& settings,
                                          const properties::ptr& statistics )
{
  using boost::adaptors::transformed;
  using namespace std::placeholders;

  /* timing */
  properties_timer t( statistics );

  std::vector<bdd> fshat;
  boost::push_back( fshat, fs | transformed( std::bind( &approximate, _1, mode, level ) ) );

  return fshat;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
