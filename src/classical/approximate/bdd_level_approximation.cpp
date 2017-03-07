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
