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

#include "esop_minimization.hpp"

#include <iomanip>
#include <list>

#include <boost/algorithm/string/join.hpp>
#include <boost/assign/std/list.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/numeric.hpp>

#include <core/io/read_pla_to_bdd.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/terminal.hpp>
#include <core/utils/timer.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/
enum decomposition_type
{
  NegativeDavio,
  PositiveDavio,
  Shannon
};

enum var_values_enum
{
  VariableNegative = 0,
  VariablePositive,
  VariableAbsent
};

std::ostream& operator<<( std::ostream& os, const cube_t& cube )
{
  for ( unsigned i = 0u; i < cube.first.size(); ++i )
  {
    os << ( cube.second[i] ? ( cube.first[i] ? "1" : "0" ) : "-" );
  }
  return os;
}

boost::dynamic_bitset<> diff_cube( const cube_t& c1, const cube_t& c2 )
{
  return ((c1.second ^ c2.second) | ((c1.second & c1.first) ^ (c2.second & c2.first)));
}

/* Returns the distance of c1 and c2. If distance is 1 and bit_pos == -1, then bit_pos stores
 * the bit position in which they differ.
 */
boost::dynamic_bitset<>::size_type compute_distance( const cube_t& c1, const cube_t& c2, int& bit_pos )
{
  boost::dynamic_bitset<> diff = diff_cube( c1, c2 );
  auto d = diff.count();
  if ( d == 1 && bit_pos == -1 )
  {
    bit_pos = diff.find_first();
  }
  return d;
}

/* This changes cube c1 at position with respect to the value of c2 at this position. */
void change( cube_t& c1, const cube_t& c2, unsigned position )
{
  if ( c1.second[position] && c2.second[position] ) /* 0, 1 -> - */
  {
    c1.first.reset( position );
    c1.second.reset( position );
  }
  else if ( !c1.second[position] ) /* -, X -> ~X */
  {
    c1.first.set( position, !c2.first[position] );
    c1.second.set( position );
  }
  else if ( !c2.second[position] ) /* X, - -> ~X */
  {
    c1.first.flip( position );
  }
}

/* Alternative implementation of change, but seems to be a tiny bit slower. */
void change_alternative( cube_t& c1, const cube_t& c2, unsigned position )
{
  bool V1 = c1.first[position];
  bool V2 = c2.first[position];
  bool C1 = c1.second[position];
  bool C2 = c2.second[position];

  c1.first.set( !V1 && !V2 && (C1 ^ C2) );
  c1.second.set( !C1 && !C2 && (V1 ^ V2) );
}

class esop_manager
{
public:
  typedef std::pair<unsigned, unsigned> cube_pair_t;
  typedef std::list<cube_pair_t> cube_pair_list_t;

  esop_manager( DdManager * cudd, bool verbose = false, unsigned capacity = 1000u )
    : cudd( cudd ),
      verbose( verbose ),
      distance_lists( 3u )
  {
    _cubes.reserve( capacity );
  }

  void add_cube( cube_t cube )
  {
    unsigned cubeid = 0u;
    std::vector<char> distances( _cubes.size() );
    int bit_pos = -1;

    for ( ; cubeid < _cubes.size(); ++cubeid )
    {
      const cube_t& c = _cubes.at( cubeid );

      /* distance-0 */
      if ( ( distances[cubeid] = compute_distance( c, cube, bit_pos ) ) == 0 )
      {
        remove_cube( cubeid );
        return;
      }
    }

    /* distance-1 */
    auto it = boost::find( distances, 1 );
    if ( it != distances.end() )
    {
      unsigned distance_one_cubeid = std::distance( distances.begin(), it );
      cube_t c = _cubes.at( distance_one_cubeid );
      change( c, cube, bit_pos );
      remove_cube( distance_one_cubeid );
      add_cube( c );
      return;
    }

    /* Add cube */
    for ( unsigned i = 0u ; i < distances.size(); ++i )
    {
      if ( distances[i] <= 4 )
      {
        distance_lists[distances[i] - 2u] += std::make_pair( i, _cubes.size() );
      }
    }

    _cubes += cube;
  }

  void remove_from_distance_list( cube_pair_list_t& l, unsigned cubeid, bool remove_first = true, bool remove_second = true )
  {
    l.remove_if( [&cubeid, &remove_first, &remove_second]( const std::pair<unsigned, unsigned>& p ) {
        return ( remove_first && p.first == cubeid ) || ( remove_second && p.second == cubeid );
      } );
    for ( auto it = l.begin(); it != l.end(); ++it )
    {
      if ( remove_first  && it->first  > cubeid ) { it->first--;  }
      if ( remove_second && it->second > cubeid ) { it->second--; }
    }
  }

  void remove_cube( unsigned cubeid )
  {
    _cubes.erase( _cubes.begin() + cubeid );

    /* remove from distance lists */
    for ( unsigned i = 0u; i < 3u; ++i )
    {
      remove_from_distance_list( distance_lists[i], cubeid );
    }
  }

  /* We know the distance when we call this function, so we do not want to recompute it */
  void get_different_positions( const cube_t& c1, const cube_t& c2, unsigned distance, std::vector<unsigned>& positions )
  {
    boost::dynamic_bitset<> diff = diff_cube( c1, c2 );
    unsigned pos;

    for ( unsigned i = 0u; i < distance; ++i )
    {
      positions += ( pos = diff.find_first() );
      diff.flip( pos );
    }
  }

  std::string pair_list_to_string( const cube_pair_list_t& l )
  {
    using boost::adaptors::transformed;

    return boost::join( l | transformed( []( const cube_pair_t& p ) {
          return boost::str( boost::format( "(%d,%d)" ) % p.first % p.second ); } ), ", " );
  }

  void get_exorlink_group( const cube_t& c1, const cube_t& c2, cube_t * tmp_cubes, unsigned group, const std::vector<unsigned>& positions )
  {
    /* distance is positions.size() */
    unsigned distance = positions.size();
    for ( unsigned i = 0u; i < distance; ++i )
    {
      tmp_cubes[i] = c1;
      for ( unsigned j = 0u; j < distance; ++j )
      {
        switch ( cube_groups[cube_group_offsets[distance - 2u] + group * distance * distance + i * distance + j] )
        {
        case 1u:
          tmp_cubes[i].first.set( positions[j], c2.first[positions[j]] );
          tmp_cubes[i].second.set( positions[j], c2.second[positions[j]] );
          break;
        case 2u:
          change( tmp_cubes[i], c2, positions[j] );
          break;
        }
      }
    }
  }

  bool leads_to_improvement( unsigned cubeid1, unsigned cubeid2, unsigned distance )
  {
    assert( cubeid1 < cubeid2 );

    using boost::adaptors::transformed;

    const cube_t& c1 = _cubes.at( cubeid1 ); /* easy access to c1 */
    const cube_t& c2 = _cubes.at( cubeid2 ); /* easy access to c2 */

    std::vector<unsigned> positions;        /* positions of different cubes in c1 and c2 */
    cube_t tmp_cubes[4];                    /* used for current cube computation */
    int improvement;                        /* store the current possible improvement */
    int bit_pos;

    get_different_positions( c1, c2, distance, positions );

    /* loop over all grous */
    for ( unsigned group = 0u; group < cube_group_count[distance - 2u]; ++group )
    {
      if ( verbose )
      {
        std::cout << "  Group: " << group << std::endl;
      }

      /* reset values */
      improvement = distance - 2;

      get_exorlink_group( c1, c2, tmp_cubes, group, positions );

      /* follow exor link */
      for ( unsigned i = 0; i < distance; ++i )
      {
        if ( verbose )
        {
          std::cout << "    " << i << ": " << tmp_cubes[i] << std::endl;
        }

        bit_pos = -1;
        for ( unsigned cubeid = 0u; cubeid < _cubes.size(); ++cubeid )
        {
          /* do not calculate distance to given cubes */
          if ( cubeid == cubeid1 || cubeid == cubeid2 ) continue;

          const cube_t& ex_cube = _cubes.at( cubeid );
          auto d = compute_distance( ex_cube, tmp_cubes[i], bit_pos );
          if ( d == 0u )
          {
            improvement -= 2;
          }
          else if ( d == 1u )
          {
            improvement -= 1;
          }
        }
      }

      /* did we find a good permutation? */
      if ( ( distance == 2u && improvement < 0 ) || ( distance >= 3u && improvement <= 0 ) )
      {
        if ( verbose )
        {
          std::cout << "    Found improvement" << std::endl;
        }

        /* remove old pair */
        remove_cube( cubeid2 );
        remove_cube( cubeid1 );

        /* add new cubes */
        for ( unsigned i = 0u; i < distance; ++i )
        {
          add_cube( tmp_cubes[i] );
        }

        return true;
      }
    }

    return false;
  }

  bool exorlink( unsigned distance )
  {
    using boost::adaptors::transformed;

    assert( distance >= 2 && distance <= 4 );

    if ( verbose )
    {
      print_banner( boost::str( boost::format( "EXOR-LINK (d = %d)" ) % distance ) );
    }

    std::vector<unsigned> positions;
    for ( const auto& p : distance_lists.at( distance - 2u ) )
    {
      if ( verbose )
      {
        std::cout << "Try to optimize with cube " << p.first << " and " << p.second << std::endl;
      }

      if ( leads_to_improvement( p.first, p.second, distance ) )
      {
        return true;
      }
    }

    return false;
  }

  inline unsigned cube_count() const
  {
    return _cubes.size();
  }

  inline unsigned literal_count() const
  {
    using boost::adaptors::transformed;

    return boost::accumulate( _cubes | transformed( []( const cube_t& c ) { return c.second.count(); } ), 0u );
  }

  inline const std::vector<cube_t>& cubes() const
  {
    return _cubes;
  }

  void print_statistics()
  {
    using boost::adaptors::indexed;
    using boost::adaptors::transformed;

    print_banner( "Statistics" );

    std::cout << "Number of cubes:    " << cube_count() << std::endl;
    std::cout << "Number of literals: " << literal_count() << std::endl;
    std::cout << "Cubes:" << std::endl;
    for ( auto it : index( _cubes ) )
    {
      std::cout << boost::format( "%4d: " ) % it.index << it.value << std::endl;
    }
    std::cout << "Distance lists:" << std::endl;
    for ( unsigned i = 0u; i < 3u; ++i )
    {
      std::cout << boost::format( "%4d: " ) % (i + 2u);
      std::cout << pair_list_to_string( distance_lists[i] ) << std::endl;
    }
  }

  DdNode * to_bdd( const cube_t& cube )
  {
    DdNode * cubef = Cudd_ReadOne( cudd ), *tmp;
    Cudd_Ref( cubef );

    for ( unsigned i = 0u; i < cube.first.size(); ++i )
    {
      if ( cube.second[i] )
      {
        tmp = Cudd_bddAnd( cudd, cubef, cube.first[i] ? Cudd_bddIthVar( cudd, i ) : Cudd_Not( Cudd_bddIthVar( cudd, i ) ) );
        Cudd_Ref( tmp );
        Cudd_RecursiveDeref( cudd, cubef );
        cubef = tmp;
      }
    }

    return cubef;
  }

  DdNode * to_bdd( const std::vector<cube_t>& cube_list )
  {
    DdNode * f = Cudd_ReadLogicZero( cudd ), * tmp;
    Cudd_Ref( f );

    for ( const auto& cube : cube_list )
    {
      DdNode * cubef = to_bdd( cube );

      tmp = Cudd_bddXor( cudd, f, cubef );
      Cudd_Ref( tmp );
      Cudd_RecursiveDeref( cudd, f );
      Cudd_RecursiveDeref( cudd, cubef );
      f = tmp;
    }

    return f;
  }

  DdNode * to_bdd()
  {
    return to_bdd( _cubes );
  }

  bool verify( DdNode * f )
  {
    DdNode * esopf = to_bdd();
    DdNode * compare = Cudd_bddXnor( cudd, f, esopf );
    Cudd_Ref( compare );
    Cudd_RecursiveDeref( cudd, esopf );

    bool equal = compare == Cudd_ReadOne( cudd );

    Cudd_RecursiveDeref( cudd, compare );

    return equal;
  }

private:
  DdManager * cudd;
  bool verbose;
  std::vector<cube_t> _cubes;
  std::vector<cube_pair_list_t> distance_lists;

  static unsigned cube_groups[];
  static unsigned cube_group_count[];
  static unsigned cube_group_offsets[];
};

/**
 * (2 0) (1 2)
 * (0 2) (2 1)
 */

/**
 * (2 0 0) (1 2 0) (1 1 2)
 * (2 0 0) (1 0 2) (1 2 1)
 * (0 2 0) (2 1 0) (1 1 2)
 * (0 2 0) (0 1 2) (2 1 1)
 * (0 0 2) (2 0 1) (1 2 1)
 * (0 0 2) (0 2 1) (2 1 1)
 */

/**
 * (2 0 0 0) (1 2 0 0) (1 1 2 0) (1 1 1 2)
 * (2 0 0 0) (1 2 0 0) (1 1 0 2) (1 1 2 1)
 * (2 0 0 0) (1 0 2 0) (1 2 1 0) (1 1 1 2)
 * (2 0 0 0) (1 0 2 0) (1 0 1 2) (1 2 1 1)
 * (2 0 0 0) (1 0 0 2) (1 2 0 1) (1 1 2 1)
 * (2 0 0 0) (1 0 0 2) (1 0 2 1) (1 2 1 1)
 * (0 2 0 0) (2 1 0 0) (1 1 2 0) (1 1 1 2)
 * (0 2 0 0) (2 1 0 0) (1 1 0 2) (1 1 2 1)
 * (0 2 0 0) (0 1 2 0) (2 1 1 0) (1 1 1 2)
 * (0 2 0 0) (0 1 2 0) (0 1 1 2) (2 1 1 1)
 * (0 2 0 0) (0 1 0 2) (2 1 0 1) (1 1 2 1)
 * (0 2 0 0) (0 1 0 2) (0 1 2 1) (2 1 1 1)
 * (0 0 2 0) (2 0 1 0) (1 2 1 0) (1 1 1 2)
 * (0 0 2 0) (2 0 1 0) (1 0 1 2) (1 2 1 1)
 * (0 0 2 0) (0 2 1 0) (2 1 1 0) (1 1 1 2)
 * (0 0 2 0) (0 2 1 0) (0 1 1 2) (2 1 1 1)
 * (0 0 2 0) (0 0 1 2) (2 0 1 1) (1 2 1 1)
 * (0 0 2 0) (0 0 1 2) (0 2 1 1) (2 1 1 1)
 * (0 0 0 2) (2 0 0 1) (1 2 0 1) (1 1 2 1)
 * (0 0 0 2) (2 0 0 1) (1 0 2 1) (1 2 1 1)
 * (0 0 0 2) (0 2 0 1) (2 1 0 1) (1 1 2 1)
 * (0 0 0 2) (0 2 0 1) (0 1 2 1) (2 1 1 1)
 * (0 0 0 2) (0 0 2 1) (2 0 1 1) (1 2 1 1)
 * (0 0 0 2) (0 0 2 1) (0 2 1 1) (2 1 1 1)
 */
unsigned esop_manager::cube_groups[] = { 2, 0, 1, 2,
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

unsigned esop_manager::cube_group_count[] = { 2u, 6u, 24u };

unsigned esop_manager::cube_group_offsets[] = { 0u, 8u, 62u };

/******************************************************************************
 * PSDKRO functions                                                           *
 ******************************************************************************/
exp_cost_t count_cubes_in_exact_psdkro( DdManager * cudd, DdNode * f, exp_cache_t& exp_cache )
{
  exp_cost_t r;

  // terminal cases
  if ( f == Cudd_ReadLogicZero( cudd ) ) return std::make_pair( PositiveDavio, 0u );
  if ( f == Cudd_ReadOne( cudd ) )       return std::make_pair( PositiveDavio, 1u );

  // in cache?
  auto it = exp_cache.find( f );
  if ( it != exp_cache.end() )
  {
    return it->second;
  }

  // get co-factors
  DdNode * f0 = Cudd_NotCond( Cudd_E( f ), Cudd_IsComplement( f ) );
  DdNode * f1 = Cudd_NotCond( Cudd_T( f ), Cudd_IsComplement( f ) );
  DdNode * f2 = Cudd_bddXor( cudd, f0, f1 );
  Cudd_Ref( f2 );

  // recursively solve subproblems
  int n0, n1, n2, nmax;
  n0 = count_cubes_in_exact_psdkro( cudd, f0, exp_cache ).second;
  n1 = count_cubes_in_exact_psdkro( cudd, f1, exp_cache ).second;
  n2 = count_cubes_in_exact_psdkro( cudd, f2, exp_cache ).second;

  // determine the mostly costly expansion
  nmax = n0 > n1 ? n0 : n1;
  nmax = n2 > nmax ? n2 : nmax;

  // choose the least costly expansion
  if      ( nmax == n0 ) r = std::make_pair( NegativeDavio, n1 + n2 );
  else if ( nmax == n1 ) r = std::make_pair( PositiveDavio, n0 + n2 );
  else                   r = std::make_pair( Shannon,       n0 + n1 );

  //Cudd_RecursiveDeref( cudd, f2 );

  // cache and return result
  return exp_cache[f] = r;
}

// TODO can we do something nicer with last_index
void generate_exact_psdkro( DdManager * cudd, DdNode * f, char * var_values, int last_index, const exp_cache_t& exp_cache, const std::function<void()>& on_cube )
{
  // terminal cases
  if ( f == Cudd_ReadLogicZero( cudd ) ) return;
  if ( f == Cudd_ReadOne( cudd ) )
  {
    using boost::adaptors::indexed;

    unsigned n = Cudd_ReadSize( cudd );
    for ( unsigned i = last_index + 1; i < n; ++i )
    {
      var_values[i] = VariableAbsent;
    }

    on_cube();
    return;
  }

  // find the best expansion by a cache lookup
  unsigned exp = exp_cache.find( f )->second.first;

  // determine the top-most variable
  int index = Cudd_NodeReadIndex( f );

  // clear intermediate variables that have not been used
  for ( int i = last_index + 1; i < index; ++i )
  {
    var_values[i] = VariableAbsent;
  }

  // get co-factors
  DdNode * f0 = Cudd_NotCond( Cudd_E( f ), Cudd_IsComplement( f ) );
  DdNode * f1 = Cudd_NotCond( Cudd_T( f ), Cudd_IsComplement( f ) );
  DdNode * f2 = Cudd_bddXor( cudd, f0, f1 );
  Cudd_Ref( f2 );

  if ( exp == PositiveDavio )
  {
    var_values[index] = VariableAbsent;
    generate_exact_psdkro( cudd, f0, var_values, index, exp_cache, on_cube );
    var_values[index] = VariablePositive;
    generate_exact_psdkro( cudd, f2, var_values, index, exp_cache, on_cube );
  }
  else if ( exp == NegativeDavio )
  {
    var_values[index] = VariableAbsent;
    generate_exact_psdkro( cudd, f1, var_values, index, exp_cache, on_cube );
    var_values[index] = VariableNegative;
    generate_exact_psdkro( cudd, f2, var_values, index, exp_cache, on_cube );
  }
  else
  {
    var_values[index] = VariableNegative;
    generate_exact_psdkro( cudd, f0, var_values, index, exp_cache, on_cube );
    var_values[index] = VariablePositive;
    generate_exact_psdkro( cudd, f1, var_values, index, exp_cache, on_cube );
  }

  Cudd_RecursiveDeref( cudd, f2 );
}

void generate_exact_psdkro( esop_manager& esop, DdManager * cudd, DdNode * f, char * var_values, int last_index, const exp_cache_t& exp_cache )
{
  generate_exact_psdkro( cudd, f, var_values, last_index, exp_cache, [&esop, &cudd, &var_values]() {
      const auto n = Cudd_ReadSize( cudd );
      boost::dynamic_bitset<> lits( n, 0u ), care( n, 0u );

      auto index = 0u;
      for ( auto it : boost::make_iterator_range( var_values, var_values + Cudd_ReadSize( cudd ) ) )
      {
        lits.set( index, it == VariablePositive );
        care.set( index, it != VariableAbsent );
        ++index;
      }

      esop.add_cube( std::make_pair( lits, care ) );
    } );
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void esop_minimization( DdManager * cudd, DdNode * f, properties::ptr settings, properties::ptr statistics )
{
  using boost::adaptors::map_keys;

  /* Settings */
  bool            verbose = get( settings, "verbose", false             );
  unsigned        runs    = get( settings, "runs",    1u                );
  bool            verify  = get( settings, "verify",  false             );
  cube_function_t on_cube = get( settings, "on_cube", cube_function_t() );

  esop_manager esop( cudd, verbose );

  /* block for timing */
  {
    properties_timer t( statistics );

    /* get initial cover using exact PSDKRO optimization */
    exp_cache_t exp_cache;
    count_cubes_in_exact_psdkro( cudd, f, exp_cache );

    char * var_values = new char[Cudd_ReadSize( cudd )];
    std::fill( var_values, var_values + Cudd_ReadSize( cudd ), VariableAbsent );
    generate_exact_psdkro( esop, cudd, f, var_values, -1, exp_cache );

    delete[] var_values;

    if ( verbose )
    {
      esop.print_statistics();
    }

    /* EXOR-LINK */
    for ( unsigned i = 0u; i < runs; ++i )
    {
      unsigned old_count, cur_count = esop.cube_count();

      do {
        old_count = cur_count;

        do {
          old_count = cur_count;

          esop.exorlink( 2u );
          esop.exorlink( 3u );
          esop.exorlink( 4u );

          cur_count = esop.cube_count();
        } while ( cur_count < old_count );

        /* last gasp */
        for ( unsigned j = 0u; j < 10u; ++j )
        {
          esop.exorlink( 4u );
        }

        cur_count = esop.cube_count();
      } while ( cur_count < old_count );
    }

    if ( verbose )
    {
      esop.print_statistics();
    }
  }

  /* pass cubes */
  if ( on_cube )
  {
    boost::for_each( esop.cubes(), on_cube );
  }

  if ( statistics )
  {
    statistics->set( "cube_count", esop.cube_count() );
    statistics->set( "literal_count", esop.literal_count() );
  }

  if ( verify )
  {
    assert( esop.verify( f ) );
  }
}

void esop_minimization( const std::string& filename, properties::ptr settings, properties::ptr statistics )
{
  BDDTable bdd;
  read_pla_to_bdd( bdd, filename );

  esop_minimization( bdd.cudd, bdd.outputs.front().second, settings, statistics );
}

dd_based_esop_optimization_func dd_based_esop_minimization_func(properties::ptr settings, properties::ptr statistics)
{
  dd_based_esop_optimization_func f = [&settings, &statistics]( DdManager * cudd, DdNode * node ) {
    return esop_minimization( cudd, node, settings, statistics );
  };
  f.init( settings, statistics );
  return f;
}

pla_based_esop_optimization_func pla_based_esop_minimization_func(properties::ptr settings, properties::ptr statistics)
{
  pla_based_esop_optimization_func f = [&settings, &statistics]( const std::string& filename ) {
    return esop_minimization( filename, settings, statistics );
  };
  f.init( settings, statistics );
  return f;
}


/******************************************************************************
 * Tests                                                                      *
 ******************************************************************************/

void test_change_performance()
{
  unsigned n = 10u;
  unsigned count = 1u << 21u;

  boost::random::mt19937 gen;
  boost::random::uniform_int_distribution<> dist( 0u, (1u << n) - 1u );

  std::vector<std::pair<cube_t, cube_t> > cubes( count );
  boost::generate( cubes, [&n, &gen, &dist]() { return std::make_pair(
                                                                      std::make_pair( boost::dynamic_bitset<>( n, dist( gen ) ), boost::dynamic_bitset<>( n, dist( gen ) ) ),
                                                                      std::make_pair( boost::dynamic_bitset<>( n, dist( gen ) ), boost::dynamic_bitset<>( n, dist( gen ) ) ) ); } );

  {
    print_timer t;

    for ( const auto& p : cubes )
    {
      cube_t c = p.first;
      change( c, p.second, 5u );
    }
  }

  {
    print_timer t;

    for ( const auto& p : cubes )
    {
      cube_t c = p.first;
      change_alternative( c, p.second, 5u );
    }
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
