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

#include "shuffle.hpp"

#include <chrono>
#include <random>

#include <boost/range/algorithm/random_shuffle.hpp>

#include <core/utils/program_options.hpp>
#include <classical/utils/aig_utils.hpp>
#include <classical/mig/mig_utils.hpp>

using namespace boost::program_options;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

using dist_t = std::uniform_int_distribution<unsigned>;

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

struct rng
{
  rng( unsigned seed ) : gen( seed ) {}

  unsigned operator()( unsigned max )
  {
    dist_t dist( 0u, max );
    return dist( gen );
  }

private:
  std::default_random_engine gen;
};

template<class C>
void shuffle( C& container, unsigned seed )
{

  rng r( seed );
  boost::random_shuffle( container, r );
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

shuffle_command::shuffle_command( const environment::ptr& env )
    : aig_mig_command( env, "Shuffles PIs and POs of AIGs and MIGs", "Shuffle current %s" )
{
  opts.add_options()
    ( "seed,s", value( &seed ), "" )
    ;
}

bool shuffle_command::before()
{
  if ( !is_set( "seed" ) )
  {
    seed = (unsigned)std::chrono::system_clock::now().time_since_epoch().count();
  }

  return true;
}

bool shuffle_command::execute_aig()
{
  shuffle( cirkit::aig_info( aig() ).inputs, seed );
  shuffle( cirkit::aig_info( aig() ).outputs, seed );

  return true;
}

bool shuffle_command::execute_mig()
{
  shuffle( cirkit::mig_info( mig() ).inputs, seed );
  shuffle( cirkit::mig_info( mig() ).outputs, seed );

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
