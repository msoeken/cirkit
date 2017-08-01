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

#include "esop_synthesis.hpp"

#include <numeric>

#include <boost/assign/std/set.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/functor.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>

#include <reversible/truth_table.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/clear_circuit.hpp>
#include <reversible/io/read_pla.hpp>

#include <misc/vec/vecInt.h>

using namespace boost::assign;

namespace cirkit
{

typedef std::vector<std::pair<binary_truth_table::cube_type, binary_truth_table::cube_type> > cubes_type;

void no_reordering( std::vector<std::pair<binary_truth_table::cube_type, binary_truth_table::cube_type> >& cubes )
{
}

struct cube_cost_sum
{
  explicit cube_cost_sum( unsigned var, bool _abs ) : var( var ), _abs( _abs ) {}

  unsigned operator()( unsigned result, const cubes_type::value_type& cube ) const
  {
    return result + ( cube.first.at( var ) ? ( ( _abs || *cube.first.at( var ) ) ? 1 : -1 ) : 0 );
  }

private:
  unsigned var;
  bool _abs;
};

struct cube_is_negative
{
  explicit cube_is_negative( unsigned var ) : var( var ) {}

  bool operator()( const cubes_type::value_type& cube ) const
  {
    return !( cube.first.at( var ) && *cube.first.at( var ) );
  }

private:
  unsigned var;
};

struct cube_sort
{
  explicit cube_sort( unsigned var ) : var( var ) {}

  bool operator()( const cubes_type::value_type& cube1, const cubes_type::value_type& cube2 ) const
  {
    return !cube_is_negative( var )( cube1 ) && cube_is_negative( var)( cube2 );
  }

private:
  unsigned var;
};

weighted_reordering::weighted_reordering()
  : alpha( 0.5 ), beta( 0.5 ) {}

weighted_reordering::weighted_reordering( float alpha, float beta )
  : alpha( alpha ), beta( beta ) {}

void weighted_reordering::reorder( cubes_type::iterator begin, cubes_type::iterator end, const std::vector<unsigned>& vars ) const
{
  if ( begin == end || !vars.size() )
  {
    return;
  }

  // find best var
  std::vector<float> costs_by_var;
  for ( unsigned var : vars )
  {
    unsigned sum1 = std::accumulate( begin, end, 0u, cube_cost_sum( var, true ) );
    unsigned sum2 = std::accumulate( begin, end, 0u, cube_cost_sum( var, false ) );

    if ( sum1 == 0u )
    {
      costs_by_var += 0;
    }
    else
    {
      costs_by_var += alpha * ( 1 / sum1 ) + beta * sum2;
    }
  }

  // maximum
  unsigned max_var_index = std::max_element( costs_by_var.begin(), costs_by_var.end() ) - costs_by_var.begin();
  std::sort( begin, end, cube_sort( vars.at( max_var_index ) ) );
  cubes_type::iterator it = std::find_if( begin, end, cube_is_negative( vars.at( max_var_index ) ) );

  std::vector<unsigned> new_vars = vars;
  new_vars.erase( new_vars.begin() + max_var_index );

  reorder( begin, it, new_vars );
  reorder( it, end, new_vars );
}

void weighted_reordering::operator()( cubes_type& cubes ) const
{
  if ( !cubes.size() )
  {
    return;
  }

  std::vector<unsigned> vars( cubes.at( 0 ).first.size() );
  boost::copy( boost::irange( 0u, (unsigned)vars.size() ), vars.begin() );

  reorder( cubes.begin(), cubes.end(), vars );
}

bool esop_synthesis( circuit& circ, const std::string& filename, properties::ptr settings, properties::ptr statistics )
{

  // Settings parsing
  const auto separate_polarities    = get( settings, "separate_polarities", false );
  const auto negative_control_lines = get( settings, "negative_control_lines", true );
  const auto share_cube_on_target   = get( settings, "share_cube_on_target", true );
  const auto reordering             = get( settings, "reordering", cube_reordering_func( weighted_reordering() ) );
  const auto garbage_name           = get( settings, "garbage_name", std::string( "--" ) );

  if ( separate_polarities && negative_control_lines )
  {
    set_error_message( statistics, "Cannot separate polarities with negative control lines enabled." );
    return false;
  }

  // Run-time measuring
  properties_timer t( statistics );

  // parse ESOP file
  binary_truth_table spec;
  read_pla_settings rp_settings;
  rp_settings.extend = false;
  std::string error_msg;
  bool r = read_pla( spec, filename, rp_settings, &error_msg );

  if (!r)
  {
    set_error_message ( statistics, error_msg );
    return false;
  }

  // empty I/O
  if ( spec.inputs().empty() )
  {
    spec.set_inputs( create_name_list( "x%d", spec.num_inputs(), 1u ) );
  }
  if ( spec.outputs().empty() )
  {
    spec.set_outputs( create_name_list( "y%d", spec.num_outputs(), 1u ) );
  }

  // check for inputs and outputs
  if ( spec.inputs().size() != spec.num_inputs() || spec.outputs().size() != spec.num_outputs() )
  {
    set_error_message( statistics, "Input and Output names are not correctly specified in ESOP file." );
    return false;
  }

  // number of variables
  unsigned n = spec.num_inputs();
  clear_circuit( circ );

  if ( separate_polarities )
  {
    circ.set_lines( n * 2 + spec.num_outputs() );

    // metadata
    std::vector<std::string> inputs( circ.lines() );
    std::copy( spec.inputs().begin(), spec.inputs().end(), inputs.begin() );
    std::fill( inputs.begin() + n, inputs.begin() + 2 * n, "1" );
    std::fill( inputs.begin() + 2 * n, inputs.end(), "0" );

    std::vector<std::string> outputs( circ.lines(), garbage_name );
    std::vector<std::string> spec_outputs = spec.outputs();
    std::copy( spec_outputs.begin(), spec_outputs.end(), outputs.begin() + 2 * n );

    std::vector<constant> constants( circ.lines() );
    std::fill( constants.begin(), constants.begin() + n, constant() );
    std::fill( constants.begin() + n, constants.begin() + 2 * n, constant( true ) );
    std::fill( constants.begin() + 2 * n, constants.end(), constant( false ) );

    std::vector<bool> garbage( circ.lines(), false );
    std::fill( garbage.begin(), garbage.begin() + 2 * n, true );

    circ.set_inputs( inputs );
    circ.set_outputs( outputs );
    circ.set_constants( constants );
    circ.set_garbage( garbage );

    // apply inverter gates
    for ( unsigned i = 0u; i < n; ++i )
    {
      append_cnot( circ, make_var( i ), n + i );
    }

    // apply gates
    for ( binary_truth_table::const_iterator it = spec.begin(); it != spec.end(); ++it )
    {
      gate::control_container controls;

      // iterate through input cube (bit by bit)
      unsigned index = 0u;
      for ( const auto& in_bit : boost::make_iterator_range( it->first ) )
      {
        if ( in_bit )
        {
          controls += make_var( ( 1u - *in_bit ) * n + index ); // considers polarity to choose line
        }
        ++index;
      }

      // iterate through output cube (bit by bit)
      index = 0u;
      for ( const auto& out_bit : boost::make_iterator_range( it->second ) )
      {
        if ( out_bit && *out_bit )
        {
          append_toffoli( circ, controls, 2 * n + index );
        }
        ++index;
      }
    }
  }
  else
  {
    // smarter approach with reusing lines
    std::vector<std::pair<binary_truth_table::cube_type, binary_truth_table::cube_type> > cubes;
    for ( binary_truth_table::const_iterator it = spec.begin(); it != spec.end(); ++it )
    {
      cubes += std::make_pair( binary_truth_table::cube_type( it->first.first, it->first.second ),
                               binary_truth_table::cube_type( it->second.first, it->second.second ) );
    }

    // only reorder with positive control lines
    if ( !negative_control_lines )
    {
      reordering( cubes );
    }

    circ.set_lines( n + spec.num_outputs() );

    // metadata
    std::vector<std::string> inputs( circ.lines() );
    std::copy( spec.inputs().begin(), spec.inputs().end(), inputs.begin() );
    std::fill( inputs.begin() + n, inputs.end(), "0" );

    std::vector<std::string> outputs( circ.lines(), garbage_name );
    std::vector<std::string> spec_outputs = spec.outputs();
    std::copy( spec_outputs.begin(), spec_outputs.end(), outputs.begin() + n );

    std::vector<constant> constants( circ.lines() );
    std::fill( constants.begin(), constants.begin() + n, constant() );
    std::fill( constants.begin() + n, constants.end(), constant( false ) );

    std::vector<bool> garbage( circ.lines(), false );
    std::fill( garbage.begin(), garbage.begin() + n, true );

    circ.set_inputs( inputs );
    circ.set_outputs( outputs );
    circ.set_constants( constants );
    circ.set_garbage( garbage );

    // apply gates
    std::vector<bool> polarity( n, true );

    for ( std::vector<std::pair<binary_truth_table::cube_type, binary_truth_table::cube_type> >::const_iterator it = cubes.begin(); it != cubes.end(); ++it )
    {
      gate::control_container controls;

      // iterate through input cube (bit by bit)
      unsigned index = 0u;
      for ( const auto& in_bit : it->first )
      {
        if ( in_bit )
        {
          if ( negative_control_lines )
          {
            controls += make_var( index, *in_bit );
          }
          else
          {
            if ( polarity.at( index ) != *in_bit )
            {
              append_not( circ, index );
              polarity.at( index ) = *in_bit;
            }
            controls += make_var( index );
          }
        }
        ++index;
      }

      // iterate through output cube (bit by bit)
      if ( !share_cube_on_target )
      {
        index = 0u;
        for ( const auto& out_bit : it->second )
        {
          if ( out_bit && *out_bit )
          {
            append_toffoli( circ, controls, n + index );
          }
          ++index;
        }
      }
      else
      {
        index = 0u;
        int first = -1;
        auto pos = circ.num_gates();

        for ( const auto& out_bit : it->second )
        {
          if ( out_bit && *out_bit )
          {
            if ( first == -1 )
            {
              append_toffoli( circ, controls, n + index );
              first = n + index;
            }
            else
            {
              insert_cnot( circ, pos, make_var( first ), n + index );
              append_cnot( circ, make_var( first ), n + index );
            }
          }
          ++index;
        }
      }
    }
  }

  return true;
}

bool esop_synthesis( circuit& circ, const gia_graph::esop_ptr& esop_cover, unsigned ninputs, unsigned noutputs, const properties::ptr& settings, const properties::ptr& statistics )
{
  const auto line_map     = get( settings, "line_map",     std::vector<unsigned>() );
  const auto no_constants = get( settings, "no_constants", false );                   /* if true, also output lines have PIs */

  properties_timer t( statistics );

  /* meta data */
  if ( line_map.empty() )
  {
    clear_circuit( circ );
    circ.set_lines( ninputs + noutputs );

    if ( !no_constants )
    {
      auto constants = circ.constants();
      std::vector<bool> garbage( circ.lines(), true );
      for ( auto i = ninputs; i < circ.lines(); ++i )
      {
        constants[i] = false;
        garbage[i] = false;
      }
      circ.set_constants( constants );
      circ.set_garbage( garbage );
    }
  }

  /* cubes */
  int i;
  abc::Vec_Int_t *vec;
  Vec_WecForEachLevel( esop_cover.get(), vec, i )
  {
    /* controls */
    gate::control_container controls;
    std::vector<unsigned> targets;
    for ( auto j = 0; j < abc::Vec_IntSize( vec ); ++j )
    {
      const auto lit = abc::Vec_IntEntry( vec, j );

      if ( lit < 0 )
      {
        const auto tgt = static_cast<int>( ninputs ) - lit - 1;
        targets.push_back( line_map.empty() ? tgt : line_map[tgt] );
      }
      else
      {
        const auto ctr = abc::Abc_Lit2Var( lit );
        controls.push_back( make_var( line_map.empty() ? ctr : line_map[ctr], !abc::Abc_LitIsCompl( lit ) ) );
      }
    }

    auto pos = circ.num_gates();

    for ( auto j = 0u; j < targets.size() - 1; ++j )
    {
      insert_cnot( circ, pos, targets.back(), targets[j] );
      insert_cnot( circ, pos++, targets.back(), targets[j] );
    }
    insert_toffoli( circ, pos, controls, targets.back() );
  }

  return true;
}

bool esop_synthesis( circuit& circ, const gia_graph& gia, const properties::ptr& settings, const properties::ptr& statistics )
{
  const auto esop = gia.compute_esop_cover();
  return esop_synthesis( circ, esop, gia.num_inputs(), gia.num_outputs(), settings, statistics );
}

bool esop_synthesis( circuit& circ, const std::vector<cube2>& cubes, unsigned ninputs, const properties::ptr& settings, const properties::ptr& statistics )
{
  const auto line_map = get( settings, "line_map", std::vector<unsigned>() );

  properties_timer t( statistics );

  /* meta data */
  const auto noutputs = 1u;
  if ( line_map.empty() )
  {
    clear_circuit( circ );
    circ.set_lines( ninputs + noutputs );

    auto constants = circ.constants();
    std::vector<bool> garbage( circ.lines(), true );
    for ( auto i = ninputs; i < circ.lines(); ++i )
    {
      constants[i] = false;
      garbage[i] = false;
    }
    circ.set_constants( constants );
    circ.set_garbage( garbage );
  }

  for ( const auto& cube : cubes )
  {
    gate::control_container controls;
    for ( auto i = 0u; i < ninputs; ++i )
    {
      if ( ( cube.mask >> i ) & 1 )
      {
        controls.push_back( make_var( line_map.empty() ? i : line_map[i], ( cube.bits >> i ) & 1 ) );
      }
    }
    append_toffoli( circ, controls, ninputs );
  }

  return true;
}

pla_blif_synthesis_func esop_synthesis_func( properties::ptr settings, properties::ptr statistics )
{
  pla_blif_synthesis_func f = [&settings, &statistics]( circuit& circ, const std::string& filename ) {
    return esop_synthesis( circ, filename, settings, statistics );
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
