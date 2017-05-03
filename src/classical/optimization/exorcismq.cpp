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

#include "exorcismq.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <queue>
#include <random>
#include <string>
#include <vector>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <core/utils/string_utils.hpp>
#include <core/utils/timer.hpp>
#include <classical/abc/abc_api.hpp>
#include <classical/abc/abc_manager.hpp>

#include <base/io/ioAbc.h>
#include <misc/vec/vecWec.h>

#define L(x) { if ( verbose ) { std::cout << x << std::endl; } }

namespace abc
{
Gia_Man_t * Eso_ManCompute( Gia_Man_t * pGia, int fVerbose, Vec_Wec_t ** pvRes );
}

namespace cirkit
{

/******************************************************************************
 * exorcismq_cube                                                             *
 * -------------------------------------------------------------------------- *
 * contains one cube                                                          *
 *                                                                            *
 * variables                                                                  *
 * - bits    : 32-bits to store literals                                      *
 * - mask    : 32-bits care set                                               *
 * - cost    : T-count cost of the cube                                       *
 * - invalid : if 1, cube is invalidated and not in cover                     *
 *                                                                            *
 * methods                                                                    *
 * - num_literals()                                                           *
 *     number of literals                                                     *
 * - positions( that )                                                        *
 *     number of different positions between this and that                    *
 * - exorlink( that, distance, pos, group )                                   *
 *     performs exorlink of this and that given distance and different        *
 *     positions pos in given group                                           *
 * - print( os, n )                                                           *
 *     prints cube to os assuming a width of n                                *
 *                                                                            *
 * notes                                                                      *
 * - cube does not store its size, it has to be known from the algorithms     *
 *   that uses the cubes, e.g., the store in which they are stored.           *
 *   Typically all cubes have the same size and there is no need to store it  *
 *   in every cube.                                                           *
 ******************************************************************************/

struct exorcismq_cube
{
  uint32_t bits = 0;
  uint32_t mask = 0;
  uint64_t cost = 0;
  uint64_t invalid = 0;

  inline uint32_t num_literals() const
  {
    return __builtin_popcount( mask );
  }

  inline uint32_t positions( const exorcismq_cube& that ) const
  {
    return ( bits ^ that.bits ) | ( mask ^ that.mask );
  }

  inline std::array<exorcismq_cube, 4> exorlink( const exorcismq_cube& that, int distance, uint32_t pos, unsigned* group ) const
  {
    uint32_t tbits, tmask;
    uint32_t tpos;

    std::array<exorcismq_cube, 4> res;

    const auto cbits = ~bits & ~that.bits;
    const auto cmask = mask ^ that.mask;

    for ( int i = 0; i < distance; ++i )
    {
      tbits = bits;
      tmask = mask;
      tpos  = pos;

      for ( int j = 0; j < distance; ++j )
      {
        /* compute position */
        const uint64_t p = tpos & -tpos;
        tpos &= tpos - 1;

        switch ( *group++ )
        {
          case 0:
            /* take from this */
            break;
          case 1:
            /* take from that */
            tbits ^= ( ( that.bits & p ) ^ tbits ) & p;
            tmask ^= ( ( that.mask & p ) ^ tmask ) & p;
            break;
          case 2:
            /* take other */
            tbits ^= ( ( cbits & p ) ^ tbits ) & p;
            tmask ^= ( ( cmask & p ) ^ tmask ) & p;
            break;
        }
      }

      res[i].bits = tbits;
      res[i].mask = tmask;
    }

    return res;
  }

  std::ostream& print( std::ostream& os, unsigned n ) const
  {
    for ( auto i = 0u; i < n; ++i )
    {
      os << ( ( ( mask >> i ) & 1 ) ? ( ( ( bits >> i ) & 1 ) ? '1' : '0' ) : '-' );
    }
    return os;
  }
};

unsigned tcount( unsigned c, unsigned n )
{
  switch ( c )
  {
  case 0u:
  case 1u:
    return 0;
  case 2u:
    return 7;
  case 3u:
    return 22;
  case 4u:
    return n >= 7 ? 28 : 52;
  default:
    if ( ( n + 1 ) / 2 >= c )
    {
      return 12 * ( c - 2 ) + 4;
    }
      else
      {
        return 24 * ( c - 3 ) + 8;
      }
  }
}

/******************************************************************************
 * exorcismq_cube_store                                                       *
 * -------------------------------------------------------------------------- *
 * stores a list of cubes and manages cube pairs                              *
 *                                                                            *
 * methods                                                                    *
 * - add_cube                                                                 *
 * - compute_pairs                                                            *
 * - has_cube                                                                 *
 * - invalidate_cube                                                          *
 * - shuffle_pairs                                                            *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

class exorcismq_cube_store
{
public:
  using cube_vec_t = std::vector<exorcismq_cube>;
  using size_type  = cube_vec_t::size_type;
  using exorcismq_cube_pair = std::tuple<unsigned, unsigned, uint64_t, uint32_t>; /* index 1, index 2, cost of pair, pos of pair */

  struct exorcismq_cube_pair_sort_by_first_id
  {
    bool operator()( const exorcismq_cube_pair& p1, const exorcismq_cube_pair& p2 ) const
    {
      return std::get<0>( p1 ) < std::get<0>( p2 );
    }
  };

  struct exorcismq_cube_pair_sort_by_second_id
  {
    bool operator()( const exorcismq_cube_pair& p1, const exorcismq_cube_pair& p2 ) const
    {
      return std::get<1>( p1 ) < std::get<1>( p2 );
    }
  };

  struct exorcismq_cube_pair_sort_by_cost
  {
    bool operator()( const exorcismq_cube_pair& p1, const exorcismq_cube_pair& p2 ) const
    {
      return std::get<2>( p1 ) > std::get<2>( p2 );
    }
  };

  using exorcismq_cube_pair_queue = std::vector<exorcismq_cube_pair>;

public:
  exorcismq_cube_store() : cube_pairs( 3u ) {}

  inline unsigned numvars() const
  {
    return n;
  }

  inline void set_numvars( unsigned _n )
  {
    n = _n;
  }

  void add_cube( exorcismq_cube& cube )
  {
    cube.cost = tcount( cube.num_literals(), n );

    cube_vec_t::const_iterator it;
    bool equal;

    std::tie( it, equal ) = has_cube( cube );
    if ( it != cubes.end() )
    {
      if ( equal )
      {
        invalidate_cube( std::distance<cube_vec_t::const_iterator>( cubes.begin(), it ) );
      }
      else /* distance 1 cube */
      {
        auto other = *it;
        invalidate_cube( std::distance<cube_vec_t::const_iterator>( cubes.begin(), it ) );

        assert( !other.invalid );
        const auto cbits = ~cube.bits & ~other.bits;
        const auto cmask = cube.mask ^ other.mask;
        const auto p = cube.positions( other );

        other.bits ^= ( ( cbits & p ) ^ other.bits ) & p;
        other.mask ^= ( ( cmask & p ) ^ other.mask ) & p;

        add_cube( other );
      }
    }
    else if ( !free_ids.empty() )
    {
      const auto id = free_ids.front();
      free_ids.pop();
      cubes[id] = cube;
    }
    else
    {
      cubes.push_back( cube );
    }
  }

  void compute_pairs()
  {
    uint64_t total = 0;

    /* clear pair cubes */
    for ( auto& p : cube_pairs )
    {
      p.clear(); // = exorcismq_cube_pair_queue();
    }

    /* generate pairs for each cube */
    for ( size_type i = 0; i < cubes.size(); ++i )
    {
      if ( !cubes[i].invalid )
      {
        total += cubes[i].cost;
        add_pairs( i );
      }
    }

    sort_cubes();
    L( boost::format( "[i] total cost: %d" ) % total );
  }

  void sort_cubes()
  {
    switch ( sorting_strategy )
    {
    case 0u:
      {
        exorcismq_cube_pair_sort_by_first_id cmp;
        for ( auto& p : cube_pairs )
        {
          std::sort( p.begin(), p.end(), cmp );
        }
      } break;
    case 1u:
      {
        exorcismq_cube_pair_sort_by_second_id cmp;
        for ( auto& p : cube_pairs )
        {
          std::sort( p.begin(), p.end(), cmp );
        }
      } break;
    case 2u:
      {
        exorcismq_cube_pair_sort_by_cost cmp;
        for ( auto& p : cube_pairs )
        {
          std::sort( p.begin(), p.end(), cmp );
        }
      } break;
    }
  }

  void set_sorting_strategy( unsigned strategy )
  {
    sorting_strategy = strategy;
    sort_cubes();
  }

  void increase_sorting_strategy()
  {
    set_sorting_strategy( ( sorting_strategy + 1 ) % 3 );
  }

  inline cube_vec_t::const_iterator begin() const
  {
    return cubes.begin();
  }

  inline cube_vec_t::const_iterator end() const
  {
    return cubes.end();
  }

  /* if second parameter is true, cube exactly matches, otherwise one bit is different */
  std::pair<cube_vec_t::const_iterator, bool> has_cube( const exorcismq_cube& cube ) const
  {
    assert( !cube.invalid );
    auto d = 2u;

    auto it = std::find_if( cubes.begin(), cubes.end(), [&cube, &d]( const exorcismq_cube& other ) {
        if ( !other.invalid )
        {
          d = __builtin_popcount( cube.positions( other ) );
          return d < 2;
        }
        return false;
      } );

    assert( it == cubes.end() || d < 2 );
    return std::make_pair( it, d == 0 );
  }

  void invalidate_cube( size_type index )
  {
    auto& cube = cubes[index];
    if ( cube.invalid ) return;

    cube.invalid = 1;
    free_ids.push( index );
  }

  inline const exorcismq_cube& operator[]( size_type index ) const
  {
    return cubes[index];
  }

  inline exorcismq_cube_pair_queue& pairs( unsigned distance )
  {
    return cube_pairs[distance - 2];
  }

  inline void shuffle_pairs()
  {
    for ( auto d = 0; d < 2; ++d )
    {
      std::shuffle( cube_pairs[d].begin(), cube_pairs[d].end(), std::default_random_engine( 0xcafe ) );
    }
  }

  template<typename F>
  void foreach_cube( F&& f )
  {
    for ( const auto& cube : cubes )
    {
      if ( !cube.invalid )
      {
        f( cube, n );
      }
    }
  }

private:
  void add_pairs( unsigned index )
  {
    auto& cube = cubes[index];
    assert( !cube.invalid );

    /* compute distances and positions to previous cubes */
    std::vector<unsigned> distances;
    std::vector<uint64_t> positions;
    distances.reserve( index );
    positions.reserve( index );

    /* we use all_of to stop when we hit a distance of 1 or below */
    auto other_id = -1;
    std::all_of( cubes.begin(), cubes.begin() + index, [this, &cube, &distances, &positions, &other_id, index]( exorcismq_cube& other ) {
        ++other_id;

        /* don't combine with invalid cube */
        if ( other.invalid )
        {
          distances.push_back( 65 );
          positions.push_back( 0 );
          return true;
        } /* continue */

        const auto p = cube.positions( other );
        const auto d = __builtin_popcount( p );

        assert( d > 1 );

        distances.push_back( d );
        positions.push_back( p );

        return true;
      } );

    /* and add all pairs according to their distance */
    for ( auto i = 0u; i < distances.size(); ++i )
    {
      const auto d = distances[i] - 2;
      if ( d <= 2 )
      {
        cube_pairs[d].emplace_back( i, index, cube.cost + cubes[i].cost, positions[i] );
      }
    }
  }

private:
  unsigned                               n;
  cube_vec_t                             cubes;
  std::queue<size_type>                  free_ids;
  std::vector<exorcismq_cube_pair_queue> cube_pairs;

  unsigned sorting_strategy = 0;

  bool verbose = true;
};

/******************************************************************************
 * manager                                                                    *
 ******************************************************************************/

class exorcismq_manager
{
public:
  exorcismq_manager( const properties::ptr& settings )
    : //cube_pairs( 3u ),
      verbose( get( settings, "verbose", verbose ) )
  {
  }

  void read_from_file( const std::string& filename )
  {
    line_parser( filename, {
        {std::regex( "^\\.i +(\\d+)$" ), [this]( const std::smatch& m ) {
            cubes.set_numvars( boost::lexical_cast<unsigned>( std::string( m[1u] ) ) );
            //n = boost::lexical_cast<unsigned>( std::string( m[1u] ) );
          }},
        {std::regex( "^([01-]+) +([01]+)$" ), [this]( const std::smatch& m ) {
            const auto scube = std::string( m[1u] );
            assert( scube.size() == cubes.numvars() );

            exorcismq_cube cube;

            for ( auto i = 0u; i < cubes.numvars(); ++i )
            {
              switch ( scube[i] )
              {
              case '0':
                cube.mask |= 1 << i;
                break;
              case '1':
                cube.bits |= 1 << i;
                cube.mask |= 1 << i;
                break;
              case '-':
                break;
              }
            }

            cubes.add_cube( cube );
          }}
      } );

    cubes.compute_pairs();
  }

  void read_from_esop_cover( abc::Vec_Wec_t * esop, unsigned num_inputs )
  {
    abc::Vec_Int_t * line;
    int c, k, lit;

    using abc::Vec_WecSize;
    using abc::Vec_WecEntry;
    using abc::Vec_IntSize;
    using abc::Vec_IntEntry;

    cubes.set_numvars( num_inputs );

    Vec_WecForEachLevel( esop, line, c )
    {
      /* single output (for now) */
      assert( abc::Vec_IntPop( line ) == -1 );

      exorcismq_cube cube;
      Vec_IntForEachEntry( line, lit, k )
      {
        cube.mask |= 1 << abc::Abc_Lit2Var( lit );

        if ( !abc::Abc_LitIsCompl( lit ) ) /* 0 */
        {
          cube.bits |= 1 << abc::Abc_Lit2Var( lit );
        }
      }

      cubes.add_cube( cube );
    }

    cubes.compute_pairs();
    write_to_file( "/tmp/before.esop" );
  }

  void write_to_file( const std::string& filename )
  {
    uint64_t total = 0;

    std::ofstream os( filename.c_str(), std::ofstream::out );

    os << ".i " << cubes.numvars() << std::endl
       << ".o 1" << std::endl
      //       << ".p " << cubes.size() << std::endl
       << ".type esop" << std::endl;
    cubes.foreach_cube( [&total, &os]( const exorcismq_cube& cube, unsigned n ) {
      total += cube.cost;
      cube.print( os, n );
      os << " 1" << std::endl;
      } );
    os << ".e" << std::endl;

    os.close();

    L( boost::format( "[i] total cost: %d" ) % total );
  }

  template<typename Fn, typename Fn_before, typename Fn_after>
  void foreach_pair_and_group( unsigned distance, Fn&& f, Fn_before&& f_before, Fn_after&& f_after )
  {
    const auto offset = cube_group_offsets[distance - 2];
    const auto skip = distance * distance;
    auto& pairs = cubes.pairs( distance );

    L( boost::format( "[i] check for %d pairs with distance %d" ) % pairs.size() % distance );

    /* iterate through pairs */
    while ( !pairs.empty() )
    {
      const auto top = pairs.back();
      pairs.pop_back();

      /* get cubes from pair */
      const auto& c1 = cubes[std::get<0>( top )];
      const auto& c2 = cubes[std::get<1>( top )];

      if ( c1.invalid || c2.invalid ) continue;

      f_before();

      /* iterate through groups */
      for ( auto i = 0u; i < cube_group_count[distance - 2]; ++i )
      {
        if ( !f( c1, c2, &cube_groups[offset + i * skip], std::get<0>( top ), std::get<1>( top ), std::get<2>( top ), std::get<3>( top ) ) ) break;
      }

      f_after();
    }
  }

  bool optimize( unsigned max_distance, bool strict = true )
  {
    std::vector<exorcismq_cube> new_cubes;

    for ( auto d = 2u; d <= max_distance; ++d )
    {
      /* check d == 4 only if there has not been improvement yet */
      //if ( d == 4u && !new_cubes.empty() ) { break; }

      foreach_pair_and_group( d, [this, d, strict, &new_cubes]( const exorcismq_cube& c1, const exorcismq_cube& c2, unsigned* group, unsigned c1_id, unsigned c2_id, uint64_t pair_cost, uint32_t positions ) {
          auto res = c1.exorlink( c2, d, positions, group );
          std::vector<exorcismq_cube_store::size_type> invalids;

          const auto cost = calculate_group_costs( res, d, invalids );

          /* cost improvement? */
          if ( cost < static_cast<int64_t>( pair_cost ) || ( !strict && ( cost == static_cast<int64_t>( pair_cost ) ) ) )
          {
            accept_group( c1_id, c2_id, res, d, invalids, new_cubes );
            return false;
          }

          return true;
        }, []() {}, []() {} );
    }

    return add_new_cubes( new_cubes );
  }

  bool optimize_with_best( unsigned max_distance )
  {
    std::vector<exorcismq_cube> new_cubes;

    int64_t best_improvement = 0;
    unsigned best_c1_id = 0, best_c2_id = 0;
    std::array<exorcismq_cube, 4> best_res;
    std::vector<exorcismq_cube_store::size_type> best_invalids;

    for ( auto d = 2u; d <= max_distance; ++d )
    {
      /* check d == 4 only if there has not been improvement yet */
      //if ( d == 4u && !new_cubes.empty() ) { break; }

      foreach_pair_and_group( d, [this, d, &new_cubes, &best_improvement, &best_c1_id, &best_c2_id, &best_res, &best_invalids]( const exorcismq_cube& c1, const exorcismq_cube& c2, unsigned* group, unsigned c1_id, unsigned c2_id, uint64_t pair_cost, uint32_t positions ) {
          auto res = c1.exorlink( c2, d, positions, group );
          std::vector<exorcismq_cube_store::size_type> invalids;

          const auto cost = calculate_group_costs( res, d, invalids );

          /* cost improvement? */
          if ( cost < static_cast<int64_t>( pair_cost ) )
          {
            int64_t improvement = pair_cost - cost;
            if ( improvement > best_improvement )
            {
              best_improvement = improvement;
              best_c1_id = c1_id;
              best_c2_id = c2_id;
              best_res = res;
              best_invalids = invalids;
            }
          }

          return true;
        }, [&best_improvement]() {
          best_improvement = -1;
        }, [this, d, &new_cubes, &best_improvement, &best_c1_id, &best_c2_id, &best_res, &best_invalids]() {
          if ( best_improvement != -1 )
          {
            accept_group( best_c1_id, best_c2_id, best_res, d, best_invalids, new_cubes );
          }
        } );
    }

    return add_new_cubes( new_cubes );
  }

  void run()
  {
    auto no_improv_round = 0u;
    auto quality = 3u;

    /* pre optimization */
    for ( auto i = 0; i < 10; ++i )
    {
      optimize( 4u );
      optimize_with_best( 4u );
    }

    do
    {
      auto improv = false;
      for ( auto i = 0; i < 8; ++i )
      {
        improv |= optimize( 3u );
        improv |= optimize_with_best( 3u );
        if ( !improv ) optimize( 3u, false );
      }

      if ( !improv )
      {
        ++no_improv_round;
      }

      if ( no_improv_round == quality )
      {
        improv |= optimize( 4u );
        improv |= optimize_with_best( 4u );

        if ( !improv )
        {
          optimize( 4u, false );
          improv |= optimize( 4u );
          improv |= optimize_with_best( 4u );
        }
      }

      if ( improv )
      {
        no_improv_round = 0u;
      }
    } while ( no_improv_round < quality );
  }

private:
  int64_t calculate_group_costs( std::array<exorcismq_cube, 4>& res, unsigned distance, std::vector<exorcismq_cube_store::size_type>& invalids )
  {
    int64_t cost = 0;

    for ( auto j = 0u; j < distance; ++j )
    {
      res[j].cost = tcount( res[j].num_literals(), cubes.numvars() );
      exorcismq_cube_store::cube_vec_t::const_iterator it;
      bool equal;
      std::tie( it, equal ) = cubes.has_cube( res[j] );

      if ( it != cubes.end() && equal )
      {
        res[j].invalid = 1;
        cost -= res[j].cost;
        invalids.push_back( std::distance<exorcismq_cube_store::cube_vec_t::const_iterator>( cubes.begin(), it ) );
        //assert( !cubes[invalids.back()].invalid );
      }
      else
      {
        cost += res[j].cost;
      }
    }

    return cost;
  }

  void accept_group( unsigned c1_id, unsigned c2_id, const std::array<exorcismq_cube, 4>& res, unsigned distance, const std::vector<exorcismq_cube_store::size_type>& invalids, std::vector<exorcismq_cube>& new_cubes )
  {
    cubes.invalidate_cube( c1_id );
    cubes.invalidate_cube( c2_id );

    for ( auto j : invalids )
    {
      assert( j != c1_id );
      assert( j != c2_id );
      cubes.invalidate_cube( j );
    }

    for ( auto j = 0u; j < distance; ++j )
    {
      if ( !res[j].invalid )
      {
        new_cubes.push_back( res[j] );
      }
    }
  }

  bool add_new_cubes( std::vector<exorcismq_cube>& new_cubes )
  {
    if ( !new_cubes.empty() )
    {
      for ( auto& cube : new_cubes )
      {
        cubes.add_cube( cube );
      }
      cubes.compute_pairs();
      return true;
    }

    cubes.compute_pairs();
    return false;
  }

  void check_exorlink_result( const exorcismq_cube_store::exorcismq_cube_pair& p,  const std::array<exorcismq_cube, 4>& res, unsigned distance )
  {
    const auto& c1 = cubes[std::get<0>( p )];
    const auto& c2 = cubes[std::get<1>( p )];

    assert( !c1.invalid );
    assert( !c2.invalid );

    //assert( c1.positions( c2 ) == std::get<3>( p ) );

    for ( auto j = 0u; j < distance; ++j )
    {
      assert( !res[j].invalid );

      if ( ( res[j].bits == c1.bits && res[j].mask == c1.mask ) ||
           ( res[j].bits == c2.bits && res[j].mask == c2.mask ) )
      {
        std::cout << "pos: " << std::get<3>( p ) << " c1: ";
        c1.print( std::cout, cubes.numvars() );
        std::cout << " c2: ";
        c2.print( std::cout, cubes.numvars() );

        for ( auto jj = 0u; jj < distance; ++jj )
        {
          std::cout << " ";
          if ( j == jj )
            std::cout << "*";
          std::cout << "res[" << jj << "]: ";
          res[j].print( std::cout, cubes.numvars() );
        }
        std::cout << std::endl;

        assert( false );
      }
    }
  }


private:
  exorcismq_cube_store                   cubes;

  bool verbose = false;

  //unsigned opt_rounds = 0u;

  static unsigned cube_groups[];
  static unsigned cube_group_count[];
  static unsigned cube_group_offsets[];
};

unsigned exorcismq_manager::cube_groups[] = { 2, 0, 1, 2,
                                              0, 2, 2, 1,
                                              2, 0, 0, 1, 2, 0, 1, 1, 2,
                                              2, 0, 0, 1, 0, 2, 1, 2, 1,
                                              0, 2, 0, 2, 1, 0, 1, 1, 2,
                                              0, 2, 0, 0, 1, 2, 2, 1, 1,
                                              0, 0, 2, 2, 0, 1, 1, 2, 1,
                                              0, 0, 2, 0, 2, 1, 2, 1, 1,
                                              2, 0, 0, 0, 1, 2, 0, 0, 1, 1, 2, 0, 1, 1, 1, 2,
                                              2, 0, 0, 0, 1, 2, 0, 0, 1, 1, 0, 2, 1, 1, 2, 1,
                                              2, 0, 0, 0, 1, 0, 2, 0, 1, 2, 1, 0, 1, 1, 1, 2,
                                              2, 0, 0, 0, 1, 0, 2, 0, 1, 0, 1, 2, 1, 2, 1, 1,
                                              2, 0, 0, 0, 1, 0, 0, 2, 1, 2, 0, 1, 1, 1, 2, 1,
                                              2, 0, 0, 0, 1, 0, 0, 2, 1, 0, 2, 1, 1, 2, 1, 1,
                                              0, 2, 0, 0, 2, 1, 0, 0, 1, 1, 2, 0, 1, 1, 1, 2,
                                              0, 2, 0, 0, 2, 1, 0, 0, 1, 1, 0, 2, 1, 1, 2, 1,
                                              0, 2, 0, 0, 0, 1, 2, 0, 2, 1, 1, 0, 1, 1, 1, 2,
                                              0, 2, 0, 0, 0, 1, 2, 0, 0, 1, 1, 2, 2, 1, 1, 1,
                                              0, 2, 0, 0, 0, 1, 0, 2, 2, 1, 0, 1, 1, 1, 2, 1,
                                              0, 2, 0, 0, 0, 1, 0, 2, 0, 1, 2, 1, 2, 1, 1, 1,
                                              0, 0, 2, 0, 2, 0, 1, 0, 1, 2, 1, 0, 1, 1, 1, 2,
                                              0, 0, 2, 0, 2, 0, 1, 0, 1, 0, 1, 2, 1, 2, 1, 1,
                                              0, 0, 2, 0, 0, 2, 1, 0, 2, 1, 1, 0, 1, 1, 1, 2,
                                              0, 0, 2, 0, 0, 2, 1, 0, 0, 1, 1, 2, 2, 1, 1, 1,
                                              0, 0, 2, 0, 0, 0, 1, 2, 2, 0, 1, 1, 1, 2, 1, 1,
                                              0, 0, 2, 0, 0, 0, 1, 2, 0, 2, 1, 1, 2, 1, 1, 1,
                                              0, 0, 0, 2, 2, 0, 0, 1, 1, 2, 0, 1, 1, 1, 2, 1,
                                              0, 0, 0, 2, 2, 0, 0, 1, 1, 0, 2, 1, 1, 2, 1, 1,
                                              0, 0, 0, 2, 0, 2, 0, 1, 2, 1, 0, 1, 1, 1, 2, 1,
                                              0, 0, 0, 2, 0, 2, 0, 1, 0, 1, 2, 1, 2, 1, 1, 1,
                                              0, 0, 0, 2, 0, 0, 2, 1, 2, 0, 1, 1, 1, 2, 1, 1,
                                              0, 0, 0, 2, 0, 0, 2, 1, 0, 2, 1, 1, 2, 1, 1, 1 };

unsigned exorcismq_manager::cube_group_count[] = { 2u, 6u, 24u };

unsigned exorcismq_manager::cube_group_offsets[] = { 0u, 8u, 62u };

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void exorcismq_minimization_from_esop( const std::string& filename, const properties::ptr& settings, const properties::ptr& statistics )
{
  const auto esopname = get( settings, "esopname", std::string( "/tmp/exorcismq.esop" ) );

  properties_timer t( statistics );

  exorcismq_manager mgr( settings );
  mgr.read_from_file( filename );

  mgr.run();

  mgr.write_to_file( esopname );
}

void exorcismq_minimization_from_blif( const std::string& filename, const properties::ptr& settings, const properties::ptr& statistics )
{
  abc_manager::get();
  auto * frame = abc::Abc_FrameGetGlobalFrame();
  abc::Cmd_CommandExecute( frame, boost::str( boost::format( "read_blif %s; &get; &w /tmp/test.aig; &exorcism" ) % filename ).c_str() );
  auto * gia = abc::Abc_FrameGetGia( frame );

  // auto * ntk = abc::Io_Read( const_cast<char*>( blifname.c_str() ), abc::IO_FILE_BLIF, 1, 0 );
  // auto * strash = abc::Abc_NtkStrash( ntk, 0, 1, 0 );
  // auto * aig = abc::Abc_NtkToDar( strash, 0, 1 );
  // auto * gia = abc::Gia_ManFromAig( aig );

  abc::Vec_Wec_t * esop = nullptr;
  abc::Eso_ManCompute( gia, 0, &esop );

  exorcismq_minimization_from_cover( esop, abc::Gia_ManCiNum( gia ), settings, statistics );

  abc::Vec_WecFree( esop );
  abc::Gia_ManStop( gia );
  // abc::Aig_ManStop( aig );
  // abc::Abc_NtkDelete( strash );
  // abc::Abc_NtkDelete( ntk );
}

void exorcismq_minimization_from_cover( abc::Vec_Wec_t * cover, unsigned num_inputs, const properties::ptr& settings, const properties::ptr& statistics )
{
  const auto esopname = get( settings, "esopname", std::string( "/tmp/exorcismq.esop" ) );

  properties_timer t( statistics );

  exorcismq_manager mgr( settings );
  mgr.read_from_esop_cover( cover, num_inputs );

  mgr.run();

  mgr.write_to_file( esopname );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
