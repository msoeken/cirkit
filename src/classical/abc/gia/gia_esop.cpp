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

#include "gia_esop.hpp"

#include <cstdint>

#include <core/utils/buckets.hpp>
#include <core/utils/flat_2d_vector.hpp>
#include <core/utils/terminal.hpp>
#include <core/utils/timer.hpp>
#include <classical/utils/cube2.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

class gia_extract_cover_manager
{
public:
  gia_extract_cover_manager( const gia_graph& gia, const properties::ptr& settings )
    : gia( gia ),
      cubes( gia.size() ),
      levels( gia.num_inputs() + 1 ),
      minimize( get( settings, "minimize", true ) ),
      progress( get( settings, "progress", false ) )
  {
  }

  gia_graph::esop_ptr run()
  {
    gia.foreach_input( [this]( int index, int i ) {
        const auto c = cube2::elementary_cube( i );
        cubes.append_singleton( index, c );
      } );

    progress_line pline( boost::str( boost::format( "[i] (gia_to_esop) inputs = %5d   gates = %%5d / %5d   last size = %%7d   total size = %%7d    runtime = %%7.2f secs" ) % gia.num_inputs() % ( gia.size() - gia.num_inputs() - gia.num_outputs() ) ), progress );
    auto counter = 0u;
    gia.foreach_and( [this, &counter, &pline]( int index, abc::Gia_Obj_t* obj ) {
        increment_timer t( &runtime );
        prepare_child( abc::Gia_ObjFaninId0( obj, index ), abc::Gia_ObjFaninC0( obj ), cubes1 );
        prepare_child( abc::Gia_ObjFaninId1( obj, index ), abc::Gia_ObjFaninC1( obj ), cubes2 );
        compute_and( index );

        pline( ++counter, cubes.size( index ), cubes.size(), runtime );
      } );

    abc::Vec_Wec_t *esop = abc::Vec_WecAlloc( 0u );
    gia.foreach_output( [this, esop]( int id, int i ) {
      const auto obj = abc::Gia_ManObj( gia, id );
      prepare_child( abc::Gia_ObjFaninId0p( gia, obj ), abc::Gia_ObjFaninC0( obj ), cubes1 );

      for ( const auto& c : cubes1 )
      {
        auto * level = abc::Vec_WecPushLevel( esop );

        for ( auto j = 0; j < gia.num_inputs(); ++j )
        {
          if ( ( c.mask >> j ) & 1 )
          {
            abc::Vec_IntPush( level, ( j << 1u ) | !( ( c.bits >> j ) & 1 ) );
          }
        }

        abc::Vec_IntPush( level, -i - 1 );
      }
    } );

    return gia_graph::esop_ptr( esop, &abc::Vec_WecFree );
  }

  std::vector<cube2> run2()
  {
    gia.foreach_input( [this]( int index, int i ) {
        const auto c = cube2::elementary_cube( i );
        cubes.append_singleton( index, c );
      } );

    progress_line pline( boost::str( boost::format( "[i] gates = %%5d / %5d   last size = %%7d   total size = %%7d    runtime = %%7.2f secs" ) % ( gia.size() - gia.num_inputs() - gia.num_outputs() ) ), progress );
    auto counter = 0u;
    gia.foreach_and( [this, &pline, &counter]( int index, abc::Gia_Obj_t* obj ) {
        prepare_child( abc::Gia_ObjFaninId0( obj, index ), abc::Gia_ObjFaninC0( obj ), cubes1 );
        prepare_child( abc::Gia_ObjFaninId1( obj, index ), abc::Gia_ObjFaninC1( obj ), cubes2 );
        compute_and( index );

        pline( ++counter, cubes.size( index ), cubes.size(), runtime );
      } );

    gia.foreach_output( [this]( int id, int i ) {
      const auto obj = abc::Gia_ManObj( gia, id );
      prepare_child( abc::Gia_ObjFaninId0p( gia, obj ), abc::Gia_ObjFaninC0( obj ), cubes1 );
    } );

    return cubes1;
  }

private:
  using cube_vec_t = std::vector<cube2>;

  void prepare_child( int index, bool cpl, cube_vec_t& v )
  {
    v.clear();
    if ( !cpl )
    {
      cubes.copy_to( index, v );
    }
    else
    {
      if ( cubes.size( index ) == 0u )
      {
        v.push_back( cube2::one_cube() );
      }
      else
      {
        auto first = cubes.at( index, 0u );
        auto offset = 0u;
        if ( first == cube2::one_cube() )
        {
          offset = 1u;
        }
        else if ( first.num_literals() == 1u )
        {
          first.invert_all();
          v.push_back( first );
          offset = 1u;
        }
        else
        {
          v.push_back( cube2::one_cube() );
        }
        cubes.copy_to( index, v, offset );
      }
    }
  }

  void compute_and( int index )
  {
    /* one of the children is 0 function */
    if ( cubes1.empty() || cubes2.empty() )
    {
      cubes.append_empty( index );
      return;
    }

    for ( const auto& c1 : cubes1 )
    {
      /* left child is 1 function */
      if ( c1 == cube2::one_cube() )
      {
        for ( const auto& c2 : cubes2 )
        {
          add_to_levels( c2 );
        }
        continue;
      }

      for ( const auto& c2 : cubes2 )
      {
        if ( c2 == cube2::one_cube() )
        {
          add_to_levels( c1 );
          continue;
        }

        auto p = c1 & c2;
        if ( p != cube2::zero_cube() )
        {
          add_to_levels( p );
        }
      }
    }

    /* add from levels */
    cube_vec_t res;
    for ( auto i = 0; i < gia.num_inputs() + 1; ++i )
    {
      std::copy( levels.begin( i ), levels.end( i ), std::back_inserter( res ) );
      levels.clear( i );
    }
    cubes.append_vector( index, res );
  }

  void add_to_levels( const cube2& c )
  {
    const auto level = c.num_literals();

    /* cancel? */
    auto index = levels.find( level, c );
    if ( index >= 0 )
    {
      levels.remove_at( level, index );
      return;
    }

    if ( c == cube2::one_cube() )
    {
      levels.add( 0u, c );
      return;
    }

    if ( minimize )
    {
      /* change one bit and hash cubes */
      for ( auto i = 0; i < gia.num_inputs(); ++i )
      {
        auto c2 = c;

        for ( auto a = 0; a < 2; ++a )
        {
          c2.rotate( i );
          const auto level2 = c2.num_literals();
          auto index2 = levels.find( level2, c2 );
          if ( index2 != -1 )
          {
            const auto new_cube = c.merge( levels.get( level2, index2 ) );
            levels.remove_at( level2, index2 );
            add_to_levels( new_cube );
            return;
          }
        }
      }
    }

    levels.add( level, c );
  }

private:
  const gia_graph& gia;
  flat_2d_vector<cube2> cubes;
  cube_vec_t cubes1;
  cube_vec_t cubes2;
  hash_buckets<cube2> levels;

  bool minimize = true;
  bool progress = false;

  double runtime = 0.0;
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

gia_graph::esop_ptr gia_extract_cover( const gia_graph& gia, const properties::ptr& settings, const properties::ptr& statistics )
{
  gia_extract_cover_manager mgr( gia, settings );
  return mgr.run();
}

std::vector<cube2> gia_extract_cover2( const gia_graph& gia, const properties::ptr& settings, const properties::ptr& statistics )
{
  gia_extract_cover_manager mgr( gia, settings );
  return mgr.run2();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
