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
    dist_t dist( 0u, max - 1 );
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
  shuffle( aig_info().inputs, seed );
  shuffle( aig_info().outputs, seed );

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
