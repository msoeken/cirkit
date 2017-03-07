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

#include "add_output_noise.hpp"

#include <chrono>
#include <map>
#include <random>
#include <set>
#include <vector>

#include <boost/format.hpp>

#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>
#include <classical/utils/aig_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

using dist_t = std::uniform_int_distribution<unsigned>;

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void insert_unique( std::vector<aig_node>& v, const aig_node& f )
{
  if ( boost::find( v, f ) == v.end() )
  {
    v.push_back( f );
  }
}

void remove_output( std::set<aig_function>& outputs, const aig_node& n )
{
  auto it = outputs.find( {n, true} );
  if ( it != outputs.end() ) { outputs.erase( it ); }

  it = outputs.find( {n, false} );
  if ( it != outputs.end() ) { outputs.erase( it ); }
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

aig_graph add_output_noise( const aig_graph& aig,
                            const properties::ptr& settings,
                            const properties::ptr& statistics )
{
  using boost::format;
  using boost::str;

  /* settings */
  const auto num_levels   = get( settings, "num_levels",   2u );
  const auto num_gates    = get( settings, "num_gates",    std::make_pair( 20u, 50u ) );
  const auto keep_outputs = get( settings, "keep_outputs", false );
  const auto seed         = get( settings, "seed",         (unsigned)std::chrono::system_clock::now().time_since_epoch().count() );
  const auto verbose      = get( settings, "verbose",      false );

  /* timing */
  properties_timer t( statistics );

  /* random generators and distributions */
  std::default_random_engine gen( seed );
  dist_t bool_dist( 0u, 1u );
  dist_t gates_dist( num_gates.first, num_gates.second );

  /* copy AIG */
  auto new_aig = aig;
  auto& info = aig_info( new_aig );

  /* maintain a list of outputs */
  std::set<aig_function> outputs;
  std::vector<aig_node>  fanin;                /* we need vector for random-access */
  std::map<aig_node, unsigned> relative_level;

  for ( const auto& p : info.outputs )
  {
    outputs.insert( p.first );
    insert_unique( fanin, p.first.node );
    relative_level.insert( {p.first.node, 0u} );
  }

  auto num_gates_to_add = gates_dist( gen );

  if ( verbose )
  {
    std::cout << format( "[i] try to add %d gates" ) % num_gates_to_add << std::endl;
  }

  for ( auto i = 0u; i < num_gates_to_add; ++i )
  {
    /* find two inputs */
    auto in1 = dist_t( 0u, fanin.size() - 2u )( gen );
    auto in2 = dist_t( 0u, fanin.size() - 1u )( gen );
    if ( in1 == in2 ) { ++in2; }

    /* find three polarities */
    auto pol_in1 = ( bool_dist( gen ) == 1u );
    auto pol_in2 = ( bool_dist( gen ) == 1u );
    auto pol_out = ( bool_dist( gen ) == 1u );

    const auto old_size = boost::num_vertices( new_aig );
    auto new_f = aig_create_and( new_aig, {fanin[in1], pol_in1}, {fanin[in2], pol_in2} );
    if ( boost::num_vertices( new_aig ) == old_size )
    {
      if ( verbose )
      {
        std::cout << "[i] skipped adding gate due to redundancy" << std::endl;
      }
      continue;
    }
    new_f.complemented ^= pol_out;

    /* New fanin is not output anymore */
    remove_output( outputs, fanin[in1] );
    remove_output( outputs, fanin[in2] );

    outputs.insert( new_f );

    auto new_level = std::max( relative_level[fanin[in1]], relative_level[fanin[in2]] ) + 1u;

    if ( verbose )
    {
      std::cout << format( "[i] added new gate %d = (%d,%d) at level %d" )
        % aig_to_literal( new_aig, new_f )
        % aig_to_literal( new_aig, {fanin[in1], pol_in1} )
        % aig_to_literal( new_aig, {fanin[in2], pol_in2} )
        % new_level << std::endl;
    }

    if ( new_level < num_levels )
    {
      insert_unique( fanin, new_f.node ); /* it is possible that node exists because of structural hashing */
      relative_level.insert( {new_f.node, new_level} );
    }
  }

  /* (maybe) remove old outputs */
  if ( !keep_outputs )
  {
    info.outputs.clear();
  }

  /* add new outputs */
  for ( const auto& o : index( outputs ) )
  {
    aig_create_po( new_aig, o.value, str( format( "r%d" ) % o.index ) );
  }

  return new_aig;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
