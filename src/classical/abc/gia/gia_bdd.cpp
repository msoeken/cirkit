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

#include "gia_bdd.hpp"

#include <vector>

#include <core/utils/terminal.hpp>
#include <core/utils/timer.hpp>

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

bdd_function_t gia_to_bdd( const gia_graph& gia, Cudd& mgr, const properties::ptr& settings, const properties::ptr& statistics )
{
  /* settings */
  const auto progress = get( settings, "progress", false );

  /* timing */
  properties_timer t( statistics );

  std::vector<BDD> node_to_bdd( gia.size() );

  /* create BDD for each input */
  gia.foreach_input( [&mgr, &node_to_bdd]( int id, int i ) {
      node_to_bdd[id] = mgr.bddVar( i );
    } );

  progress_line pline( boost::str( boost::format( "[i] (gia_to_bdd) inputs = %5d   gates = %%5d / %5d   BDD nodes = %%9d   runtime = %%7.2f secs" ) % gia.num_inputs() % ( gia.size() - gia.num_inputs() - gia.num_outputs() ) ), progress );
  double runtime = 0.0;
  auto counter = 0u;

  /* create BDD for each AND gate */
  gia.foreach_and( [&runtime, &pline, &counter, &mgr, &node_to_bdd]( int id, abc::Gia_Obj_t* obj ) {
      increment_timer t( &runtime );
      const auto b1 = node_to_bdd[abc::Gia_ObjFaninId0( obj, id )];
      const auto b2 = node_to_bdd[abc::Gia_ObjFaninId1( obj, id )];
      const auto c1 = abc::Gia_ObjFaninC0( obj );
      const auto c2 = abc::Gia_ObjFaninC1( obj );

      node_to_bdd[id] = ( c1 ? !b1 : b1 ) & ( c2 ? !b2 : b2 );

      pline( ++counter, mgr.ReadKeys(), runtime );
    } );

  /* create BDD for each output */
  std::vector<BDD> outputs;
  gia.foreach_output( [&gia, &node_to_bdd, &outputs]( int id, int i ) {
      const auto obj = abc::Gia_ManObj( gia, id );
      const auto b = node_to_bdd[abc::Gia_ObjFaninId0p( gia, obj )];
      const auto c = abc::Gia_ObjFaninC0( obj );

      outputs.push_back( c ? !b : b );
    } );

  return {mgr, outputs};
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
