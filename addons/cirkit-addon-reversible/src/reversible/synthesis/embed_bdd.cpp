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

#include "embed_bdd.hpp"

#include <vector>

#include <boost/assign/std/vector.hpp>

#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>

#include <reversible/functions/calculate_additional_lines.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

struct garbage_manager
{
public:
  garbage_manager( const rcbdd& cf ) : cf( cf )
  {
    std::vector<BDD> garbage;

    for ( auto i = cf.num_outputs(); i < cf.num_vars(); ++i )
    {
      garbage += cf.y(i);
    }

    store += garbage;
  }

  const std::vector<BDD>& operator[]( unsigned index )
  {
    if ( index >= store.size() )
    {
      for ( auto i = store.size(); i <= index; ++i )
      {
        store += dec( store[i - 1u] );
      }
    }

    return store.at( index );
  }

  std::vector<BDD> dec( const std::vector<BDD>& vars )
  {
    std::vector<BDD> outputs;

    for ( auto i = 0u; i < vars.size(); ++i )
    {
      auto cube = cf.manager().bddOne();

      for ( auto j = 0u; j < i; ++j )
      {
        cube &= !vars.at(j);
      }

      outputs += vars.at(i) ^ cube;
    }

    return outputs;
  }

private:
  const rcbdd& cf;
  std::vector<std::vector<BDD>> store;
};

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

bool embed_bdd( rcbdd& cf, const bdd_function_t& bdd,
                const properties::ptr& settings,
                const properties::ptr& statistics )
{
  /* timer */
  properties_timer t( statistics );

  /* some statistics */
  const unsigned n = bdd.first.ReadSize();
  const unsigned m = bdd.second.size();

  /* compute number of additional lines */
  const auto cal_settings = std::make_shared<properties>();
  const auto cal_statistics = std::make_shared<properties>();
  const auto additional_lines = calculate_additional_lines( bdd, cal_settings, cal_statistics );

  const auto r = additional_lines + n;

  const auto constant_lines = r - n;
  const auto garbage_lines  = r - m;

  /* initialize RCBDD */
  cf.initialize_manager();
  cf.create_variables( r );
  cf.set_num_inputs( n );
  cf.set_num_outputs( m );
  cf.set_input_labels( create_name_list( "x%d", n, 1u ) );
  cf.set_output_labels( create_name_list( "y%d", m, 1u ) );

  /* copy bdd to cf */
  std::vector<unsigned> index_map( n );
  for ( auto i = 0u; i < n; ++i )
  {
    index_map[i] = ( constant_lines + i ) * 3u;
  }
  const auto copied = bdd_copy( bdd.first, bdd.second, cf.manager(), index_map );

  /* create non-embedded characteristic function */
  auto small_chi = cf.manager().bddOne();

  for ( const auto& f : index( copied ) )
  {
    small_chi &= cf.y( f.index ).Xnor( f.value );
  }

  /* create embedded characteristic function */
  auto chi = cf.manager().bddZero();

  auto constants = cf.manager().bddOne();
  for ( auto i = 0u; i < constant_lines; ++i )
  {
    constants &= !cf.x( i );
  }

  garbage_manager garbage( cf );

  for ( const auto& pattern : cal_statistics->get<std::vector<std::string>>( "patterns" ) )
  {
    auto patternf = cf.manager().bddOne();

    for ( auto c : index( pattern ) )
    {
      assert( c.value == '0' || c.value == '1' );
      patternf &= ( c.value == '0' ) ? !cf.y( c.index ) : cf.y( c.index );
    }

    /* restrict small_chi */
    const auto restr = small_chi & patternf;

    /* mu */
    auto mu = 0ul;

    /* iterate through each cube */
    DdGen *gen;
    int *cube;
    CUDD_VALUE_TYPE value;
    Cudd_ForeachCube( cf.manager().getManager(), restr.getNode(), gen, cube, value )
    {
      auto cubef = constants & patternf;
      std::vector<BDD> dont_cares;

      /* inputs */
      for ( auto i = constant_lines; i < r; ++i )
      {
        if ( cube[3u * i] == 0 )
        {
          cubef &= !cf.x( i );
        }
        else if ( cube[3u * i] == 1 )
        {
          cubef &= cf.x( i );
        }
        else
        {
          dont_cares += cf.x( i );
        }
      }

      /* garbage */
      auto dec_garbage = garbage[mu];

      for ( auto i = 0u; i < garbage_lines; ++i )
      {
        if ( i < dont_cares.size() )
        {
          cubef &= dec_garbage.at( i ).Xnor( dont_cares.at( dont_cares.size() - i - 1u ) );
        }
        else
        {
          cubef &= !dec_garbage.at( i );
        }
      }

      chi |= cubef;
      mu += ( 1ul << dont_cares.size() );
    }
  }

  cf.set_chi( chi );

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
