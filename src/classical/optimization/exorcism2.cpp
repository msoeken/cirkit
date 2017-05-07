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
  exorcism2_manager( const std::vector<cube2>& original, int num_vars )
    : cubes( num_vars + 1 ),
      num_vars( num_vars ),

      pairs( 2u ),
      pairs_tmp( 5u )
  {
    for ( auto i = 2; i <= 4; ++i )
    {
      pairs.push_back( boost::circular_buffer<std::pair<cube2, cube2>>( original.size() * original.size() ) );
    }

    for ( const auto& c : original )
    {
      add_cube( c );
    }

    for ( auto i = 2; i <= 4; ++i )
    {
      std::cout << boost::format( "[i] distance %d cubes: %d" ) % i % pairs[i].size() << std::endl;
    }
    std::cout << boost::format( "[i] number of cubes: %d\n" ) % cubes.size() << std::endl;
  }

  std::vector<cube2> run()
  {
    bool improv{};
    unsigned rounds = 0u;

    do
    {
      improv = exorlink2();
      improv = exorlink3() || improv;
      improv = exorlink2();
      improv = exorlink3() || improv;
      improv = exorlink2();
      improv = exorlink3() || improv;
      improv = exorlink2();
      improv = exorlink3() || improv;
      improv = exorlink2();
      improv = exorlink3() || improv;
      improv = exorlink2();
      improv = exorlink3() || improv;

      if ( improv )
      {
        rounds = 0u;
      }
      else
      {
        ++rounds;
      }
    } while ( rounds <= 2u );

    improv_lits = true;

    do
    {
      improv = exorlink2();
      improv = exorlink3() || improv;

      if ( improv )
      {
        rounds = 0u;
      }
      else
      {
        ++rounds;
      }
    } while ( rounds < 2u );

    for ( auto i = 2; i <= 4; ++i )
    {
      std::cout << boost::format( "[i] distance %d cubes: %d" ) % i % pairs[i].size() << std::endl;
    }

    std::vector<cube2> res;

    for ( auto i = 0; i <= num_vars; ++i )
    {
      std::copy( cubes.begin( i ), cubes.end( i ), std::back_inserter( res ) );
    }

    return res;
  }

private:
  int add_cube( const cube2& c )
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
    int imp;
    if ( ( imp = pair_with_others( c, lits ) ) >= 0 ) return imp + 1;
    for ( auto d = 1; d <= 3; ++d )
    {
      if ( lits >= d && ( ( imp = pair_with_others( c, lits - d ) ) >= 0 ) ) return imp + 1;
      if ( lits <= num_vars - d && ( ( imp = pair_with_others( c, lits + d ) ) >= 0 ) ) return imp + 1;
    }

    /* 3. no 1-distance cube found, insert cube and copy pairs */
    if ( cubes.is_dry() ) return 0;

    cubes.add( lits, c );
    for ( auto d = 2; d <= 4; ++d )
    {
      std::copy( pairs_tmp[d].begin(), pairs_tmp[d].end(), std::back_inserter( pairs[d] ) );
    }

    return 0;
  }

  int add_cube_dry( const cube2& c )
  {
    cubes.start_dry();
    const auto r = add_cube( c );
    cubes.stop_dry();
    return r;
  }

  int pair_with_others( const cube2& c, unsigned level )
  {
    for ( auto it = cubes.begin( level ); it != cubes.end( level ); ++it )
    {
      const auto d = c.distance( *it );
      if ( d == 1 )
      {
        const auto new_cube = c.merge( *it );
        cubes.remove_at( level, std::distance( cubes.begin( level ), it ) );
        return add_cube( new_cube );
      }

      if ( d <= 3 )
      {
        pairs_tmp[d].push_back( std::make_pair( c, *it ) );
      }
    }

    return -1;
  }

  bool exorlink2()
  {
    auto improv = false;

    auto& ps = pairs[2u];

    // std::sort( ps.begin(), ps.end(), []( const std::pair<cube2, cube2>& a, const std::pair<cube2, cube2>& b ) {
    //     return a.first.num_literals() + a.second.num_literals() < b.first.num_literals() + b.second.num_literals();
    //   } );

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

      auto n = p.first.exorlink( p.second, 2, p.first.differences( p.second ), &cube_groups2[0u] );

      if ( add_cube_dry( n[0] ) || add_cube_dry( n[1] ) )
      {
        add_cube( n[0] );
        add_cube( n[1] );

        improv = true;
      }
      else
      {
        n = p.first.exorlink( p.second, 2, p.first.differences( p.second ), &cube_groups2[4u] );

        if ( add_cube_dry( n[0] ) || add_cube_dry( n[1] ) ||
             ( improv_lits && ( ( n[0].num_literals() + n[1].num_literals() ) < ( c1_size + c2_size ) ) ) )
        {
          add_cube( n[0] );
          add_cube( n[1] );

          improv = true;
        }
        else
        {
          cubes.add( c1_size, p.first );
          cubes.add( c2_size, p.second );
          ps.push_back( p );
        }
      }
    }

    return improv;
  }

  bool exorlink3()
  {
    auto improv = false;

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

      auto found = false;
      for ( auto g = 0u; g < 54u; g += 9u )
      {
        const auto n = p.first.exorlink( p.second, 3, p.first.differences( p.second ), &cube_groups3[g] );
        auto gain = 0u;

        for ( auto j = 0u; j < 3u; ++j )
        {
          gain += add_cube_dry( n[j] );
          if ( gain > 1 )
          {
            for ( auto k = 0u; k < 3u; ++k )
            {
              add_cube( n[k] );
            }
            improv = true;
            found = true;
            break;
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

    return improv;
  }

private:
  hash_backtrack_buckets<cube2> cubes;
  int num_vars;

  std::vector<boost::circular_buffer<std::pair<cube2, cube2>>> pairs;
  std::vector<std::vector<std::pair<cube2, cube2>>> pairs_tmp;

  bool improv_lits = false;

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

  exorcism2_manager mgr( cubes, num_vars );
  return mgr.run();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
