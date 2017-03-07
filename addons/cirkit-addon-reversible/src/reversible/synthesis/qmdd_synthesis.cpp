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

#include "qmdd_synthesis.hpp"

#include <iostream>

#include <core/utils/timer.hpp>
#include <classical/optimization/optimization.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/functions/add_gates.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

class qmdd_synthesis_manager
{
public:
  qmdd_synthesis_manager( circuit& circ, const rcbdd& cf )
    : circ( circ ), cf( cf )
  {
    copy_meta_data( circ, cf );
  }

  void run()
  {
    auto chi = cf.chi();
    for ( auto var = 0u; var < cf.num_vars(); ++var )
    {
      chi = adjust_variable( chi, var );
    }
  }

private:
  BDD adjust_variable( const BDD& chi, unsigned var );
  void create_toffoli_gates( unsigned var, const BDD& gate );

private:
  circuit&     circ;
  const rcbdd& cf;

public:
  dd_based_esop_optimization_func esopmin;
  bool                            verbose;
};

BDD qmdd_synthesis_manager::adjust_variable( const BDD& chi, unsigned var )
{
  /* cubes for later */
  auto *cube1 = new char[3u * cf.num_vars()];
  auto *cube2 = new char[3u * cf.num_vars()];

  auto new_chi = chi;

  while ( true )
  {
    /* get 0->0 and 0->1 mappings */
    auto map00 = cf.cofactor( new_chi, var, false, false );
    auto map01 = cf.cofactor( new_chi, var, false, true );

    /* get output parts of mappings */
    map00 = cf.remove_xs( map00 ).ExistAbstract( cf.y( var ) );
    map01 = cf.remove_xs( map01 ).ExistAbstract( cf.y( var ) );

    /* done? */
    if ( map01 == cf.manager().bddZero() ) { break; }

    /* get distinct output patterns */
    const auto distinct = cf.move_ys_to_xs( map01 & !map00 );

    /* create STG */
    const auto stg = cf.create_from_gate( var, distinct );
    create_toffoli_gates( var, distinct );
    new_chi = cf.compose( new_chi, stg );

    /* remove previous variables */
    auto prev = cf.manager().bddOne();
    for ( auto y = 0u; y < var; ++y )
    {
      prev &= cf.y( y );
    }

    /* are there non distinct cubes */
    const auto non_distinct = ( map01 & map00 ).ExistAbstract( prev );
    const auto non_map00    = ( !map00 ).ExistAbstract( prev );

    if ( non_distinct == cf.manager().bddZero() ) { break; }

    /* pick one cube from non_distinct */
    non_distinct.PickOneCube( cube1 );
    non_map00.PickOneCube( cube2 );

    for ( auto y = var + 1; y < cf.num_vars(); ++y )
    {
      auto idx = 3u * y + 1u;
      if ( cube1[idx] == 2 ) { cube1[idx] = 0; }
      if ( cube2[idx] == 2 ) { cube2[idx] = 0; }

      if ( cube1[idx] != cube2[idx] )
      {
        auto& g = prepend_cnot( circ, make_var( var, true ), y );
        new_chi = cf.compose( new_chi, cf.create_from_gate( g ) );
      }
    }
  }

  /* clean up */
  delete[] cube1;
  delete[] cube2;

  return new_chi;
}

void qmdd_synthesis_manager::create_toffoli_gates( unsigned var, const BDD& gate )
{
  if ( gate == cf.manager().bddZero() ) return;

  esopmin.settings()->set( "on_cube", cube_function_t( [&]( const cube_t& c ) {
        gate::control_container controls;
        for ( auto i = 0u; i < cf.num_vars(); ++i )
        {
          if ( c.second[3u * i] )
          {
            controls += make_var( i, c.first[3u * i] );
          }
        }

        prepend_toffoli( circ, controls, var );
      } ) );
  esopmin.settings()->set( "verify", false );
  esopmin( gate.manager(), gate.getNode() );
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

bool qmdd_synthesis( circuit& circ, const rcbdd& cf,
                     const properties::ptr& settings,
                     const properties::ptr& statistics )
{
  /* settings */
  const auto verbose = get( settings, "verbose", false );
  const auto esopmin = get( settings, "esopmin", dd_based_esop_optimization_func() );

  /* timining */
  properties_timer t( statistics );

  qmdd_synthesis_manager mgr( circ, cf );
  mgr.esopmin = esopmin;
  mgr.verbose = verbose;
  mgr.run();

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
