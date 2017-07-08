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

#include "embed_truth_table.hpp"

#include <boost/assign/std/vector.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

#include <core/utils/timer.hpp>

#include <reversible/functions/extend_truth_table.hpp>
#include <reversible/functions/truth_table_from_bitset.hpp>

#include "synthesis_utils_p.hpp"

using namespace boost::assign;

// NOTE use unsigned long long instead of unsigned
// now this approach works only for embedding with 32 extra lines
namespace cirkit
{

unsigned additional_garbage( const binary_truth_table& base, std::vector<unsigned>& values )
{
  std::map<unsigned, unsigned> output_value_count;

  for ( binary_truth_table::const_iterator it = base.begin(); it != base.end(); ++it )
  {
    binary_truth_table::cube_type out( it->second.first, it->second.second );
    unsigned number = truth_table_cube_to_number( out );

    if ( output_value_count.find( number ) == output_value_count.end() )
    {
      output_value_count.insert( std::make_pair( number, 1 ) );
    }
    else
    {
      ++output_value_count[number];
    }
  }

  // copy the highest count to mu, e.g. ([0,4], [1,3]) -> 4
  unsigned mu = boost::max_element( output_value_count,
                                  []( const std::map<unsigned,unsigned>::value_type& a, const std::map<unsigned,unsigned>::value_type& b ) {
                                    return a.second < b.second;
                                  } )->second;

  using boost::adaptors::map_keys;
  boost::push_back( values, output_value_count | map_keys ); // TODO map_values here instead?

  return (unsigned)ceil( log( (double)mu ) / log( 2.0 ) );
}

struct minimal_hamming_distance
{
  explicit minimal_hamming_distance( unsigned compare_to ) : compare_to( compare_to ) {}

  bool operator()( unsigned x, unsigned y ) const
  {
    return hamming_distance( compare_to, x ) < hamming_distance( compare_to, y );
  }

private:
  unsigned compare_to;
};

bool embed_truth_table( binary_truth_table& spec, const binary_truth_table& base, const properties::ptr& settings, const properties::ptr& statistics )
{
  std::string garbage_name           = get<std::string>( settings, "garbage_name", "g" );
  std::vector<unsigned> output_order = get<std::vector<unsigned> >( settings, "output_order", std::vector<unsigned>() );

  properties_timer t( statistics );

  /* get number of additional garbage lines needed */
  std::vector<unsigned> values;
  unsigned ag = additional_garbage( base, values );
  ag = (unsigned)std::max( (int)ag, (int)base.num_inputs() - (int)base.num_outputs() );
  unsigned cons = base.num_outputs() + ag - base.num_inputs();

  /* we don't need more than that */
  assert( base.num_inputs() <= base.num_outputs() + ag );

  /* new number of bits */
  std::map<unsigned, unsigned> new_spec;
  unsigned new_bw = base.num_outputs() + ag;

  /* all outputs which are left */
  std::vector<unsigned> all_outputs( 1u << new_bw );
  std::copy( boost::make_counting_iterator( 0u ), boost::make_counting_iterator( 1u << new_bw ), all_outputs.begin() );

  {
    /* greedy method */
    std::map<unsigned,std::vector<unsigned> > output_assignments;

    /* output order */
    if ( output_order.size() != base.num_outputs() )
    {
      output_order.clear();
      std::copy( boost::make_counting_iterator( 0u ), boost::make_counting_iterator( base.num_outputs() ), std::back_inserter( output_order ) );
    }

    /* not in output order for filling up */
    std::vector<unsigned> left_positions;
    for ( unsigned order = 0u; order < new_bw; ++order )
    {
      if ( std::find( output_order.begin(), output_order.end(), order ) == output_order.end() )
      {
        left_positions += order;
      }
    }

    /* since you append the garbage to the end, just count the numbers */
    for ( std::vector<unsigned>::const_iterator itValue = values.begin(); itValue != values.end(); ++itValue )
    {
      unsigned base = 0u;
      for ( unsigned j = 0u; j < output_order.size(); ++j )
      {
        unsigned bit_in_value = !!( *itValue & ( 1u << ( output_order.size() - 1u - j ) ) );
        unsigned bit_pos_in_base = new_bw - 1u - output_order.at( j );
        base |= ( bit_in_value << bit_pos_in_base );
      }

      /* create the values from (value)0..0 to (value)1..1 with rebaset to output_order */
      std::vector<unsigned> assignments;

      for ( unsigned j = 0u; j < ( 1u << ag ); ++j )
      {
        unsigned assignment = base;
        for ( unsigned k = 0; k < ag; ++k )
        {
          unsigned bit_in_value = !!( j & ( 1u << ( ag - 1u - k ) ) );
          unsigned bit_pos_in_base = new_bw - 1u - left_positions.at( k );
          assignment |= ( bit_in_value << bit_pos_in_base );
        }
        assignments += assignment;
      }

      output_assignments[*itValue] = assignments;
    }

    /* truth table is in order */
    for ( binary_truth_table::const_iterator it = base.begin(); it != base.end(); ++it )
    {
      /* input value */
      binary_truth_table::cube_type in( it->first.first, it->first.second );
      unsigned number_in = truth_table_cube_to_number( in );

      /* output value */
      binary_truth_table::cube_type out( it->second.first, it->second.second );
      unsigned number = truth_table_cube_to_number( out );

      /* best suiting element */
      std::vector<unsigned>::iterator bestFit = std::min_element( output_assignments[number].begin(), output_assignments[number].end(), minimal_hamming_distance( number_in ) );
      new_spec.insert( std::make_pair( number_in, *bestFit ) );

      /* remove element from list */
      all_outputs.erase( std::find( all_outputs.begin(), all_outputs.end(), *bestFit ) );
      output_assignments[number].erase( bestFit );
    }
  }

  for ( unsigned i = 0u; i < ( 1u << new_bw ); ++i )
  {
    if ( new_spec.find( i ) == new_spec.end() )
    {
      std::vector<unsigned>::iterator bestFit = std::min_element( all_outputs.begin(), all_outputs.end(), minimal_hamming_distance( i ) );
      new_spec.insert( std::make_pair( i, *bestFit ) );
      all_outputs.erase( bestFit );
    }
  }

  spec.clear();

  for ( std::map<unsigned, unsigned>::const_iterator it = new_spec.begin(); it != new_spec.end(); ++it )
  {
    spec.add_entry( number_to_truth_table_cube( it->first, new_bw ),
                    number_to_truth_table_cube( it->second, new_bw ) );
  }

  /* meta-data */
  std::vector<std::string> inputs( new_bw, "i" );
  std::fill( inputs.begin(), inputs.begin() + cons, "0" );
  std::copy( base.inputs().begin(), base.inputs().end(), inputs.begin() + cons );
  spec.set_inputs( inputs );

  std::vector<std::string> outputs( new_bw, garbage_name );
  for ( std::vector<unsigned>::iterator it = output_order.begin(); it != output_order.end(); ++it ) {
    unsigned index = std::distance( output_order.begin(), it );
    if ( index < base.outputs().size() )
    {
      outputs.at( *it ) = base.outputs().at( index );
    }
  }
  spec.set_outputs( outputs );

  std::vector<constant> constants( new_bw );
  std::fill( constants.begin(), constants.begin() + cons, false );
  std::fill( constants.begin() + cons, constants.end(), constant() );
  spec.set_constants( constants );

  std::vector<bool> garbage( new_bw, true );
  for ( std::vector<unsigned>::const_iterator it = output_order.begin(); it != output_order.end(); ++it ) {
    garbage.at( *it ) = false;
  }
  spec.set_garbage( garbage );

  return true;
}

bool embed_truth_table( binary_truth_table& spec, const tt& base, const properties::ptr& settings, const properties::ptr& statistics )
{
  const auto rbase = truth_table_from_bitset_direct( base );
  return embed_truth_table( spec, rbase, settings, statistics );
}

embedding_func embed_truth_table_func( const properties::ptr& settings, const properties::ptr& statistics )
{
  embedding_func f = [&settings, &statistics]( binary_truth_table& spec, const binary_truth_table& base ) {
    return embed_truth_table( spec, base, settings, statistics );
  };
  f.init( settings, statistics );
  return f;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
