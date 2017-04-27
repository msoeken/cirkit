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

#include <core/utils/flat_2d_vector.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

class cube
{
public:
  cube()
    : value( 0 )
  {
  }

  cube( uint32_t bits, uint32_t mask )
    : bits( bits ), mask( mask )
  {
  }

  inline int distance( const cube& that ) const
  {
    return __builtin_popcount( ( bits ^ that.bits ) | ( mask ^ that.mask ) );
  }

  inline int num_literals() const
  {
    return __builtin_popcount( mask );
  }

  inline bool operator==( const cube& that ) const
  {
    return value == that.value;
  }

  inline bool operator!=( const cube& that ) const
  {
    return value != that.value;
  }

  inline cube operator&( const cube& that ) const
  {
    /* literals must agree on intersection */
    const auto int_mask = mask & that.mask;
    if ( ( bits ^ that.bits ) & int_mask )
    {
      return zero_cube();
    }

    return cube( bits | that.bits, mask | that.mask );
  }

  /* it is assumed that this and that have distance 1 */
  inline cube merge( const cube& that ) const
  {
    const auto d = ( bits ^ that.bits ) | ( mask ^ that.mask );
    return cube( bits ^ ( ~that.bits & d ), mask ^ ( that.mask & d ) );
  }

  /* invert all literals */
  inline void invert_all()
  {
    bits ^= mask;
  }

  static inline cube elementary_cube( unsigned index )
  {
    const auto bits = 1 << index;
    return cube( bits, bits );
  }

  void print( unsigned length = 32, std::ostream& os = std::cout ) const
  {
    for ( auto i = 0u; i < length; ++i )
    {
      os << ( ( ( mask >> i ) & 1 ) ? ( ( ( bits >> i ) & 1 ) ? '1' : '0' ) : '-' );
    }
  }

  static inline cube one_cube()
  {
    return cube( 0, 0 );
  }

  static inline cube zero_cube()
  {
    return cube( ~0, 0 );
  }

  union
  {
    struct
    {
      uint32_t bits;
      uint32_t mask;
    };
    uint64_t value;
  };
};

class gia_extract_cover_manager
{
public:
  gia_extract_cover_manager( const gia_graph& gia )
    : gia( gia ),
      cubes( gia.size() ),
      levels( gia.num_inputs() + 1 )
  {
  }

  gia_graph::esop_ptr run()
  {
    gia.foreach_input( [this]( int index, int i ) {
        const auto c = cube::elementary_cube( i );
        cubes.append_singleton( index, c );
      } );

    gia.foreach_and( [this]( int index, abc::Gia_Obj_t* obj ) {
        prepare_child( abc::Gia_ObjFaninId0( obj, index ), abc::Gia_ObjFaninC0( obj ), cubes1 );
        prepare_child( abc::Gia_ObjFaninId1( obj, index ), abc::Gia_ObjFaninC1( obj ), cubes2 );
        compute_and( index );
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

private:
  using cube_vec_t = std::vector<cube>;

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
        v.push_back( cube::one_cube() );
      }
      else
      {
        auto first = cubes.at( index, 0u );
        auto offset = 0u;
        if ( first == cube::one_cube() )
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
          v.push_back( cube::one_cube() );
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
      if ( c1 == cube::one_cube() )
      {
        for ( const auto& c2 : cubes2 )
        {
          add_to_levels( c2 );
        }
        continue;
      }

      for ( const auto& c2 : cubes2 )
      {
        if ( c2 == cube::one_cube() )
        {
          add_to_levels( c1 );
          continue;
        }

        auto p = c1 & c2;
        if ( p != cube::zero_cube() )
        {
          add_to_levels( p );
        }
      }
    }

    /* add from levels */
    cube_vec_t res;
    for ( auto& v : levels )
    {
      std::copy( v.begin(), v.end(), std::back_inserter( res ) );
      v.clear();
    }
    cubes.append_vector( index, res );
  }

  void add_to_levels( const cube& c )
  {
    const auto level = c.num_literals();

    /* cancel? */
    auto& vl = levels[level];
    const auto it = std::find( vl.begin(), vl.end(), c );
    if ( it != vl.end() )
    {
      vl.erase( it );
      return;
    }

    if ( c == cube::one_cube() )
    {
      levels[0u].push_back( c );
      return;
    }

    if ( minimize && level < gia.num_inputs() )
    {
      auto& vl2 = levels[level + 1];
      for ( auto it = vl2.begin(); it != vl2.end(); ++it )
      {
        if ( c.distance( *it ) == 1 )
        {
          const auto new_cube = c.merge( *it );
          vl2.erase( it );
          add_to_levels( new_cube );
          return;
        }
      }
    }

    if ( minimize )
    {
      for ( auto it = vl.begin(); it != vl.end(); ++it )
      {
        if ( c.distance( *it ) == 1 )
        {
          const auto new_cube = c.merge( *it );
          vl.erase( it );
          add_to_levels( new_cube );
          return;
        }
      }
    }

    if ( minimize && level > 0 )
    {
      auto& vl2 = levels[level - 1];
      for ( auto it = vl2.begin(); it != vl2.end(); ++it )
      {
        if ( c.distance( *it ) == 1 )
        {
          const auto new_cube = c.merge( *it );
          vl2.erase( it );
          add_to_levels( new_cube );
          return;
        }
      }
    }

    levels[level].push_back( c );
  }

private:
  const gia_graph& gia;
  flat_2d_vector<cube> cubes;
  cube_vec_t cubes1;
  cube_vec_t cubes2;
  std::vector<cube_vec_t> levels;

  bool minimize = true;
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

gia_graph::esop_ptr gia_extract_cover( const gia_graph& gia, const properties::ptr& settings, const properties::ptr& statistics )
{
  gia_extract_cover_manager mgr( gia );
  return mgr.run();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
