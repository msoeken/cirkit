/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "generate_exact_psdkro.hpp"

#include <iomanip>
#include <list>

#include <boost/algorithm/string/join.hpp>
#include <boost/assign/std/list.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

#include <core/io/read_pla_to_bdd.hpp>
#include <core/utils/timer.hpp>

using namespace boost::assign;

namespace revkit
{

/******************************************************************************
 * Utils                                                                      *
 ******************************************************************************/
void print_banner( const std::string& caption, unsigned width = 40u )
{
  std::cout << '+' << std::string( width - 2u, '-' ) << '+' << std::endl
            << "| " << std::left << std::setw( width - 3u ) << caption << '|' << std::endl
            << '+' << std::string( width - 2u, '-' ) << '+' << std::endl;
}

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

typedef std::pair<unsigned, unsigned> exp_cost_t;
typedef std::map<DdNode*, exp_cost_t> exp_cache_t;
typedef std::pair<boost::dynamic_bitset<>, boost::dynamic_bitset<> > cube_t;

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
      distance_lists( 3u ),
      link_groups( 3u )
  {
    cubes.reserve( capacity );

    link_groups[0u].resize( 2u );
    boost::for_each( link_groups[0u], []( std::vector<std::vector<unsigned> >& v ) { v.resize( 2u ); } );
    link_groups[0u][0u][0u] += 0u,2u;
    link_groups[0u][0u][1u] += 2u,1u;
    link_groups[0u][1u][0u] += 1u,2u;
    link_groups[0u][1u][1u] += 2u,0u;

    link_groups[1u].resize( 6u );


    link_groups[2u].resize( 24u );
  }

  void add_cube( cube_t cube )
  {
    unsigned cubeid = 0u;
    std::vector<char> distances( cubes.size() );
    int bit_pos = -1;

    for ( ; cubeid < cubes.size(); ++cubeid )
    {
      const cube_t& c = cubes.at( cubeid );

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
      cube_t c = cubes.at( distance_one_cubeid );
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
        distance_lists[distances[i] - 2u] += std::make_pair( i, cubes.size() );
      }
    }

    cubes += cube;
  }

  bool remove_from_distance_list( cube_pair_list_t& l, unsigned cubeid, bool remove_first = true, bool remove_second = true )
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

  bool remove_cube( unsigned cubeid )
  {
    cubes.erase( cubes.begin() + cubeid );

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

  bool leads_to_improvement( unsigned cubeid1, unsigned cubeid2, unsigned distance )
  {
    assert( cubeid1 < cubeid2 );

    using boost::adaptors::transformed;

    const cube_t& c1 = cubes.at( cubeid1 ); /* easy access to c1 */
    const cube_t& c2 = cubes.at( cubeid2 ); /* easy access to c2 */

    std::vector<unsigned> positions;        /* positions of different cubes in c1 and c2 */
    cube_t tmp_cubes[4];                    /* used for current cube computation */
    int improvement;                        /* store the current possible improvement */
    std::vector<cube_t> new_cubes;          /* new cubes */
    int bit_pos;

    get_different_positions( c1, c2, distance, positions );

    /* for now we only consider distance = 2 */
    assert( distance == 2u );

    /* loop over all permutations of the positions vector */
    do
    {
      if ( verbose )
      {
        std::cout << "  Permutation: " << boost::join( positions | transformed( boost::lexical_cast<std::string, unsigned> ), ", " ) << std::endl;
      }

      /* reset values */
      improvement = distance - 2;
      tmp_cubes[0u] = c1;
      tmp_cubes[1u] = c2;
      new_cubes.clear();

      change( tmp_cubes[0u], c2, positions.at( 0u ) );
      change( tmp_cubes[1u], c1, positions.at( 1u ) );

      new_cubes += tmp_cubes[0u],tmp_cubes[1u];

      /* follow exor link */
      for ( unsigned i = 0; i < distance; ++i )
      {
        if ( verbose )
        {
          std::cout << "    " << i << ": " << new_cubes.at( i ) << std::endl;
        }

        bit_pos = -1;
        for ( unsigned cubeid = 0u; cubeid < cubes.size(); ++cubeid )
        {
          /* do not calculate distance to given cubes */
          if ( cubeid == cubeid1 || cubeid == cubeid2 ) continue;

          const cube_t& ex_cube = cubes.at( cubeid );
          auto d = compute_distance( ex_cube, new_cubes.at( i ), bit_pos );
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
      if ( improvement < 0 )
      {
        if ( verbose )
        {
          std::cout << "    Found improvement" << std::endl;
        }

        /* remove old pair */
        remove_cube( cubeid2 );
        remove_cube( cubeid1 );

        /* add new cubes */
        for ( const auto& c : new_cubes )
        {
          add_cube( c );
        }

        return true;
      }
    } while ( boost::next_permutation( positions ) );

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
        break;
      }
    }

    return false;
  }

  void print_statistics()
  {
    using boost::adaptors::indexed;
    using boost::adaptors::transformed;

    print_banner( "Statistics" );

    std::cout << "Number of cubes: " << cubes.size() << std::endl;
    std::cout << "Cubes:" << std::endl;
    auto range = cubes | indexed( 0u );
    for ( auto it = range.begin(); it != range.end(); ++it )
    {
      std::cout << boost::format( "%4d: " ) % it.index() << *it << std::endl;
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
    return to_bdd( cubes );
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
  std::vector<cube_t> cubes;
  std::vector<cube_pair_list_t> distance_lists;
  std::vector<std::vector<std::vector<std::vector<unsigned> > > > link_groups; // distance -> group -> cube -> id
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/
exp_cost_t count_cubes_in_exact_psdkro( DdManager * cudd, DdNode * f, exp_cache_t& exp_cache, unsigned indentation )
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
  n0 = count_cubes_in_exact_psdkro( cudd, f0, exp_cache, indentation + 1 ).second;
  n1 = count_cubes_in_exact_psdkro( cudd, f1, exp_cache, indentation + 1 ).second;
  n2 = count_cubes_in_exact_psdkro( cudd, f2, exp_cache, indentation + 1 ).second;

  // determine the mostly costly expansion
  nmax = n0 > n1 ? n0 : n1;
  nmax = n2 > nmax ? n2 : nmax;

  // choose the least costly expansion
  if      ( nmax == n0 ) r = std::make_pair( NegativeDavio, n1 + n2 );
  else if ( nmax == n1 ) r = std::make_pair( PositiveDavio, n0 + n2 );
  else                   r = std::make_pair( Shannon,       n0 + n1 );

  // cache and return result
  return exp_cache[f] = r;
}

// TODO can we do something nicer with last_index
void generate_exact_psdkro( esop_manager& esop, DdManager * cudd, DdNode * f, char * var_values, int last_index, const exp_cache_t& exp_cache, const generate_exact_psdkro_settings& settings )
{
  // terminal cases
  if ( f == Cudd_ReadLogicZero( cudd ) ) return;
  if ( f == Cudd_ReadOne( cudd ) )
  {
    using boost::adaptors::indexed;

    unsigned n = Cudd_ReadSize( cudd );
    for ( int i = last_index + 1; i < n; ++i )
    {
      var_values[i] = VariableAbsent;
    }
    boost::dynamic_bitset<> lits( n, 0u ), care( n, 0u );

    auto range = boost::make_iterator_range( var_values, var_values + Cudd_ReadSize( cudd ) ) | indexed( 0u );
    for ( auto it = range.begin(); it != range.end(); ++it )
    {
      lits.set( it.index(), *it == VariablePositive );
      care.set( it.index(), *it != VariableAbsent );
    }

    esop.add_cube( std::make_pair( lits, care ) );
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
    generate_exact_psdkro( esop, cudd, f0, var_values, index, exp_cache, settings );
    var_values[index] = VariablePositive;
    generate_exact_psdkro( esop, cudd, f2, var_values, index, exp_cache, settings );
  }
  else if ( exp == NegativeDavio )
  {
    var_values[index] = VariableAbsent;
    generate_exact_psdkro( esop, cudd, f1, var_values, index, exp_cache, settings );
    var_values[index] = VariableNegative;
    generate_exact_psdkro( esop, cudd, f2, var_values, index, exp_cache, settings );
  }
  else
  {
    var_values[index] = VariableNegative;
    generate_exact_psdkro( esop, cudd, f0, var_values, index, exp_cache, settings );
    var_values[index] = VariablePositive;
    generate_exact_psdkro( esop, cudd, f1, var_values, index, exp_cache, settings );
  }

  Cudd_RecursiveDeref( cudd, f2 );
}

void generate_exact_psdkro( esop_manager& esop, DdManager * cudd, DdNode * f, const generate_exact_psdkro_settings& settings )
{
  using boost::adaptors::map_keys;

  exp_cache_t exp_cache;
  count_cubes_in_exact_psdkro( cudd, f, exp_cache, 0u );

  char * var_values = new char[Cudd_ReadSize( cudd )];
  std::fill( var_values, var_values + Cudd_ReadSize( cudd ), VariableAbsent );
  generate_exact_psdkro( esop, cudd, f, var_values, -1, exp_cache, settings );

  boost::for_each( exp_cache | map_keys, [&cudd]( DdNode* node ) { Cudd_RecursiveDeref( cudd, node ); } );
  delete var_values;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

generate_exact_psdkro_settings::generate_exact_psdkro_settings()
  : verbose( false )
{}

void generate_exact_psdkro( const std::string& filename, const generate_exact_psdkro_settings& settings )
{
  BDDTable bdd;
  read_pla_to_bdd( bdd, filename );

  esop_manager esop( bdd.cudd, settings.verbose );

  generate_exact_psdkro( esop, bdd.cudd, bdd.outputs.front().second, settings );

  if ( settings.verbose )
  {
    esop.print_statistics();
  }

  for ( unsigned i = 0u; i < 50u; ++i )
  {
    esop.exorlink( 2u );
  }

  if ( settings.verbose )
  {
    esop.print_statistics();
  }
  std::cout << "Equal? " << esop.verify( bdd.outputs.front().second ) << std::endl;
}

/******************************************************************************
 * Tests                                                                      *
 ******************************************************************************/

void test_change_performance()
{
  unsigned n = 10u;
  unsigned count = 1u << 21u;

  boost::random::mt19937 gen;
  boost::random::uniform_int_distribution<> dist( 0, (1u << n) - 1u );

  std::vector<std::pair<cube_t, cube_t> > cubes( count );
  boost::generate( cubes, [&n, &gen, &dist]() { return std::make_pair(
                                                                      std::make_pair( boost::dynamic_bitset<>( n, dist( gen ) ), boost::dynamic_bitset<>( n, dist( gen ) ) ),
                                                                      std::make_pair( boost::dynamic_bitset<>( n, dist( gen ) ), boost::dynamic_bitset<>( n, dist( gen ) ) ) ); } );

  {
    print_timer pt( std::cout );
    timer<print_timer> t( pt );

    for ( const auto& p : cubes )
    {
      cube_t c = p.first;
      change( c, p.second, 5u );
    }
  }

  {
    print_timer pt( std::cout );
    timer<print_timer> t( pt );

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
// End:
