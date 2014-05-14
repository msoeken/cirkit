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

#include <list>

#include <boost/algorithm/string/join.hpp>
#include <boost/assign/std/list.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

#include <core/io/read_pla_to_bdd.hpp>

using namespace boost::assign;

namespace revkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/
enum decomposition_type
{
  Shannon,
  PositiveDavio,
  NegativeDavio
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
boost::dynamic_bitset<>::size_type distance( const cube_t& c1, const cube_t& c2, int& bit_pos )
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

class esop_manager
{
public:
  esop_manager( bool verbose = false, unsigned capacity = 1000u )
    : verbose( verbose ),
      distance_lists( 3u )
  {
    cubes.reserve( capacity );
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
      if ( ( distances[cubeid] = distance( c, cube, bit_pos ) ) == 0 )
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

  bool remove_cube( unsigned cubeid )
  {
    cubes.erase( cubes.begin() + cubeid );

    /* remove from distance lists */
    for ( unsigned i = 0u; i < 3u; ++i )
    {
      distance_lists[i].remove_if( [&cubeid]( const std::pair<unsigned, unsigned>& p ) { return p.first == cubeid || p.second == cubeid; } );
      for ( auto it = distance_lists[i].begin(); it != distance_lists[i].end(); ++it )
      {
        if ( it->first  > cubeid ) { it->first--;  }
        if ( it->second > cubeid ) { it->second--; }
      }
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

  bool optimize( unsigned distance )
  {
    using boost::adaptors::transformed;

    assert( distance >= 2 && distance <= 4 );

    std::vector<unsigned> positions;
    for ( const auto& p : distance_lists.at( distance - 2u ) )
    {
      if ( verbose )
      {
        std::cout << "Try to optimize with cube " << p.first << " and " << p.second << std::endl;
      }

      /* store */
      const cube_t& c1 = cubes.at( p.first );
      const cube_t& c2 = cubes.at( p.second );

      positions.clear();
      get_different_positions( c1, c2, distance, positions );

      do
      {
        if ( verbose )
        {
          std::cout << "Permutation: " << boost::join( positions | transformed( boost::lexical_cast<std::string, unsigned> ), ", " ) << std::endl;
        }

        cube_t c = c1;
        for ( unsigned pos : positions )
        {
          change( c, c2, pos );
          std::cout << c << std::endl;
        }
      } while ( boost::next_permutation( positions ) );
    }

    return false;
  }

  void print_statistics()
  {
    using boost::adaptors::indexed;
    using boost::adaptors::transformed;

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
      std::cout << boost::join(
                    distance_lists[i] | transformed( []( const std::pair<unsigned, unsigned>& p ) {
                        return boost::str( boost::format( "(%d,%d)" ) % p.first % p.second ); } ), ", " )
                << std::endl;
    }
  }

private:
  bool verbose;
  std::vector<cube_t> cubes;
  std::vector<std::list<std::pair<unsigned, unsigned> > > distance_lists;
};

/******************************************************************************
 * Private functions                                                          *
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

  // TODO clean f2

  // determine the mostly costly expansion
  nmax = n0 > n1 ? n0 : n1;
  nmax = n2 > nmax ? n2 : nmax;

  // choose the least costly expansion
  if      ( nmax == n0 ) r = std::make_pair( PositiveDavio, n1 + n2 );
  else if ( nmax == n1 ) r = std::make_pair( NegativeDavio, n0 + n2 );
  else                   r = std::make_pair( Shannon,       n0 + n1 );

  // cache and return result
  return exp_cache[f] = r;
}

void generate_exact_psdkro( esop_manager& esop, DdManager * cudd, DdNode * f, char * var_values, const exp_cache_t& exp_cache, const generate_exact_psdkro_settings& settings )
{
  // terminal cases
  if ( f == Cudd_ReadLogicZero( cudd ) ) return;
  if ( f == Cudd_ReadOne( cudd ) )
  {
    using boost::adaptors::indexed;

    unsigned n = Cudd_ReadSize( cudd );
    boost::dynamic_bitset<> lits( n ), care( n );

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

  // get co-factors
  DdNode * f0 = Cudd_NotCond( Cudd_E( f ), Cudd_IsComplement( f ) );
  DdNode * f1 = Cudd_NotCond( Cudd_T( f ), Cudd_IsComplement( f ) );
  DdNode * f2 = Cudd_bddXor( cudd, f0, f1 );
  Cudd_Ref( f2 );

  if ( exp == PositiveDavio )
  {
    var_values[index] = VariableAbsent;
    generate_exact_psdkro( esop, cudd, f0, var_values, exp_cache, settings );
    var_values[index] = VariablePositive;
    generate_exact_psdkro( esop, cudd, f2, var_values, exp_cache, settings );
  }
  else if ( exp == NegativeDavio )
  {
    var_values[index] = VariableAbsent;
    generate_exact_psdkro( esop, cudd, f1, var_values, exp_cache, settings );
    var_values[index] = VariableNegative;
    generate_exact_psdkro( esop, cudd, f2, var_values, exp_cache, settings );
  }
  else
  {
    var_values[index] = VariableNegative;
    generate_exact_psdkro( esop, cudd, f0, var_values, exp_cache, settings );
    var_values[index] = VariablePositive;
    generate_exact_psdkro( esop, cudd, f1, var_values, exp_cache, settings );
  }
}

void generate_exact_psdkro( esop_manager& esop, DdManager * cudd, DdNode * f, const generate_exact_psdkro_settings& settings )
{
  using boost::adaptors::map_keys;

  if ( settings.verbose )
  {
    std::cout << "Generate PSDKRO for: " << std::endl;
    Cudd_PrintMinterm( cudd, f );
  }

  exp_cache_t exp_cache;
  count_cubes_in_exact_psdkro( cudd, f, exp_cache );

  char * var_values = new char[Cudd_ReadSize( cudd )];
  std::fill( var_values, var_values + Cudd_ReadSize( cudd ), VariableAbsent );
  generate_exact_psdkro( esop, cudd, f, var_values, exp_cache, settings );

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
  esop_manager esop( settings.verbose );

  read_pla_to_bdd( bdd, filename );
  generate_exact_psdkro( esop, bdd.cudd, bdd.outputs.front().second, settings );

  esop.optimize( 2u );

  esop.print_statistics();
}

}

// Local Variables:
// c-basic-offset: 2
// End:
