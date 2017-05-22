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

#include "exorcism2.hpp"

#include <boost/circular_buffer.hpp>
#include <boost/format.hpp>

#include <core/utils/buckets.hpp>
#include <core/utils/terminal.hpp>
#include <core/utils/timer.hpp>

namespace cirkit
{

class print_nano_timer : public boost::timer::cpu_timer
{
public:
  print_nano_timer()
  {
    start();
  }

  ~print_nano_timer()
  {
    if ( !is_stopped() )
    {
      stop();
      std::cout << elapsed().wall << std::endl;
    }
  }
};

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

class exorcism2_manager
{
public:
  exorcism2_manager( const std::vector<cube2>& original, int num_vars, const properties::ptr& settings )
    : cubes( num_vars + 1 ),
      num_vars( num_vars ),
      init_cubes_size( original.size() ),

      pairs( 2u ),
      pairs_tmp( 5u ),

      progress( get( settings, "progress", false ) )
  {
    for ( auto i = 2; i <= 4; ++i )
    {
      pairs.push_back( boost::circular_buffer<std::pair<cube2, cube2>>( original.size() * original.size() ) );
    }

    for ( const auto& c : original )
    {
      add_cube( c );
    }

    print_stats();
  }

  std::vector<cube2> run()
  {
    unsigned gain{};
    unsigned rounds = 0u;

    max_dist = 3u;

    unsigned iteration = 0;
    double runtime = 0.0;

    progress_line p( "[i] exorcism   iter = %3d   i/o = %2d/1   cubes = %6d/%6d   total = %6.2f", progress );

    do
    {
      increment_timer t( &runtime );
      p( ++iteration, num_vars, cubes.size(), init_cubes_size, runtime );

      do
      {
        gain  = exorlink2();
        gain += exorlink3();
        gain += exorlink2();
        gain += exorlink3();
        gain += exorlink2();
        if ( !( gain += exorlink3() ) ) break;
        if ( !( gain += exorlink2() ) ) break;
        if ( !( gain += exorlink3() ) ) break;
        if ( !( gain += exorlink2() ) ) break;
        if ( !( gain += exorlink3() ) ) break;
        if ( !( gain += exorlink2() ) ) break;
        if ( !( gain += exorlink3() ) ) break;
      } while ( false );

      while ( rounds == 1u && !gain )
      {
        improv_lits = true;
        if ( gain += exorlink2() ) break;
        if ( gain += exorlink3() ) break;
        if ( gain += exorlink2() ) break;
        if ( gain += exorlink3() ) break;
        if ( gain += exorlink2() ) break;
        if ( gain += exorlink3() ) break;
        if ( gain += exorlink2() ) break;
        if ( gain += exorlink3() ) break;
        if ( gain += exorlink2() ) break;
        if ( gain += exorlink3() ) break;
        if ( gain += exorlink2() ) break;
        if ( gain += exorlink3() ) break;
        improv_lits = false; break;
      }

      while ( rounds == 2u && !gain )
      {
        reshape = true;
        if ( gain += exorlink2() ) break;
        if ( gain += exorlink3() ) break;
        if ( gain += exorlink2() ) break;
        if ( gain += exorlink3() ) break;
        if ( gain += exorlink2() ) break;
        if ( gain += exorlink3() ) break;
        if ( gain += exorlink2() ) break;
        if ( gain += exorlink3() ) break;
        if ( gain += exorlink2() ) break;
        if ( gain += exorlink3() ) break;
        if ( gain += exorlink2() ) break;
        if ( gain += exorlink3() ) break;
        reshape = false; break;
      }

      if ( gain > 0 )
      {
        rounds = 0u;
      }
      else
      {
        ++rounds;
      }
    } while ( rounds <= 2u );

    std::vector<cube2> res;

    for ( auto i = 0; i <= num_vars; ++i )
    {
      std::copy( cubes.begin( i ), cubes.end( i ), std::back_inserter( res ) );
    }

    return res;
  }

private:
  int add_cube( const cube2& c, bool add = true )
  {
    const auto lits = c.num_literals();

    /* 1. check if cube exists */
    const auto index = cubes.find( lits, c );
    if ( index != -1 )
    {
      cubes.remove_at( lits, index );
      return 2;
    }

    /* clear temporary pairs */
    for ( auto& v : pairs_tmp )
    {
      v.clear();
    }

    /* 2. check for 1-distance cubes and prepare pairs */
    int imp{};
    for ( auto i = std::max( lits - max_dist, 0 ); i <= std::min( num_vars, lits + max_dist ); ++i )
    {
      if ( ( imp = pair_with_others( c, i ) ) >= 0 ) return imp + 1;
    }

    /* 3. no 1-distance cube found, insert cube and copy pairs */
    if ( !add ) return 0;
    cubes.add( lits, last_added = c );
    for ( auto d = 2; d <= max_dist; ++d )
    {
      std::copy( pairs_tmp[d].begin(), pairs_tmp[d].end(), std::back_inserter( pairs[d] ) );
    }

    return 0;
  }

  int pair_with_others( const cube2& c, unsigned level )
  {
    for ( auto it = cubes.begin( level ); it != cubes.end( level ); ++it )
    {
      const auto d = c.distance( *it );
      if ( d == 1 )
      {
        const auto new_cube = c.merge( *it );
        last_removed = *it;
        cubes.remove_at( level, std::distance( cubes.begin( level ), it ) );
        saved_lits = c.num_literals() == static_cast<int>( level ) ? 1 : 0;
        return add_cube( new_cube );
      }

      if ( d <= max_dist )
      {
        pairs_tmp[d].push_back( std::make_pair( c, *it ) );
      }
    }

    return -1;
  }

  unsigned exorlink2()
  {
    const auto old_size = cubes.size();

    auto& ps = pairs[2u];

    const auto num_pairs = ps.size();

    int c1_size{}, c2_size{};

    for ( auto i = 0u; i < num_pairs; ++i )
    {
      const auto p = ps.front();
      ps.pop_front();

      c1_size = p.first.num_literals();
      if ( cubes.find( c1_size, p.first ) == -1 ) continue;
      c2_size = p.second.num_literals();
      if ( cubes.find( c2_size, p.second ) == -1 ) continue;

      /* remove c1 and c2 for now */
      cubes.remove( c1_size, p.first );
      cubes.remove( c2_size, p.second );

      const auto diff = p.first.differences( p.second );
      auto n = p.first.exorlink( p.second, 2, diff, &cube_groups2[0u] );

      if ( add_cube( n[0], false ) )
      {
        add_cube( n[1] );
      }
      else if ( add_cube( n[1], false ) )
      {
        add_cube( n[0] );
      }
      else
      {
        n = p.first.exorlink( p.second, 2, diff, &cube_groups2[4u] );

        if ( add_cube( n[0], false ) )
        {
          add_cube( n[1] );
        }
        else if ( add_cube( n[1], false ) )
        {
          add_cube( n[0] );
        }
        else if ( ( improv_lits && ( ( n[0].num_literals() + n[1].num_literals() ) < ( c1_size + c2_size ) ) ) ||
                  ( reshape && ( ( n[0].num_literals() + n[1].num_literals() ) == ( c1_size + c2_size ) ) ) )
        {
          add_cube( n[0] );
          add_cube( n[1] );
        }
        else
        {
          cubes.add( c1_size, p.first );
          cubes.add( c2_size, p.second );
          ps.push_back( p );
        }
      }
    }

    return old_size - cubes.size();
  }

  unsigned exorlink3()
  {
    const auto old_size = cubes.size();

    auto& ps = pairs[3u];
    const auto num_pairs = ps.size();

    int c1_size{}, c2_size{};

    // std::sort( ps.begin(), ps.end(), []( const std::pair<cube2, cube2>& a, const std::pair<cube2, cube2>& b ) {
    //     return a.first.num_literals() + a.second.num_literals() < b.first.num_literals() + b.second.num_literals();
    //   } );

    for ( auto i = 0u; i < num_pairs; ++i )
    {
      const auto p = ps.front();
      ps.pop_front();

      c1_size = p.first.num_literals();
      if ( cubes.find( c1_size, p.first ) == -1 ) continue;
      c2_size = p.second.num_literals();
      if ( cubes.find( c2_size, p.second ) == -1 ) continue;

      /* remove c1 and c2 for now */
      cubes.remove( c1_size, p.first );
      cubes.remove( c2_size, p.second );

      const auto diff = p.first.differences( p.second );
      auto found = false;
      for ( auto g = 0u; g < 54u; g += 9u )
      {
        const auto n = p.first.exorlink( p.second, 3, diff, &cube_groups3[g] );

        for ( auto j = 0u; j < 3u; ++j )
        {
          const auto gain = add_cube( n[j], false );

          if ( gain > 1 )
          {
            for ( auto k = 0u; k < 3u; ++k )
            {
              if ( j != k ) add_cube( n[k] );
            }
            found = true;
            break;
          }
          else if ( gain == 1 )
          {
            const auto n1 = j == 0 ? n[1] : n[0];
            const auto n2 = j == 2 ? n[1] : n[2];

            const auto _last_added = last_added;
            const auto _last_removed = last_removed;
            const auto _saved_lits = saved_lits;

            if ( add_cube( n1, false ) )
            {
              add_cube( n2 );
              found = true;
              break;
            }
            else if ( add_cube( n2, false ) )
            {
              add_cube( n1 );
              found = true;
              break;
            }
            else if ( ( improv_lits && ( ( n1.num_literals() + n2.num_literals() - _saved_lits ) < ( c1_size + c2_size ) ) ) ||
                      ( reshape && ( ( n1.num_literals() + n2.num_literals() - _saved_lits ) == ( c1_size + c2_size ) ) ) )
            {
              add_cube( n1 );
              add_cube( n2 );
              found = true;
              break;
            }
            else
            {
              cubes.remove( _last_added.num_literals(), _last_added );
              cubes.add( _last_removed.num_literals(), _last_removed );
            }
          }
        }

        if ( found ) break;
      }

      if ( !found )
      {
        cubes.add( c1_size, p.first );
        cubes.add( c2_size, p.second );
        ps.push_back( p );
      }
    }

    return old_size - cubes.size();
  }

private:
  void print_stats() const
  {
    for ( auto i = 2; i <= 4; ++i )
    {
      std::cout << boost::format( "[i] distance %d cubes: %d" ) % i % pairs[i].size() << std::endl;
    }
    std::cout << boost::format( "[i] number of cubes: %d\n" ) % cubes.size() << std::endl;
  }

private:
  hash_buckets<cube2> cubes;
  int num_vars;
  int init_cubes_size;

  std::vector<boost::circular_buffer<std::pair<cube2, cube2>>> pairs;
  std::vector<std::vector<std::pair<cube2, cube2>>> pairs_tmp;

  /* bookkeeping */
  cube2 last_added;
  cube2 last_removed;
  int saved_lits = 0;

  /* control algorithm */
  int max_dist = 2;
  bool improv_lits = false;
  bool reshape = false;

  bool progress = false;

  unsigned cube_groups2[8] = {2, 0, 1, 2,
                              0, 2, 2, 1};

  unsigned cube_groups3[54] = {2, 0, 0, 1, 2, 0, 1, 1, 2,
                               2, 0, 0, 1, 0, 2, 1, 2, 1,
                               0, 2, 0, 2, 1, 0, 1, 1, 2,
                               0, 2, 0, 0, 1, 2, 2, 1, 1,
                               0, 0, 2, 2, 0, 1, 1, 2, 1,
                               0, 0, 2, 0, 2, 1, 2, 1, 1};
};

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

std::vector<cube2> exorcism2( const std::vector<cube2>& cubes, int num_vars, const properties::ptr& settings, const properties::ptr& statistics )
{
  properties_timer t( statistics );

  exorcism2_manager mgr( cubes, num_vars, settings );
  return mgr.run();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
