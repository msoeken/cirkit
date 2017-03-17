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

#include "simplify.hpp"

#include <cmath>

#include <boost/dynamic_bitset.hpp>
#include <boost/range/algorithm_ext/iota.hpp>

#include <core/cube.hpp>
#include <core/utils/bitset_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>
#include <classical/optimization/exorcism_minimization.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/copy_circuit.hpp>
#include <reversible/functions/copy_metadata.hpp>
#include <reversible/functions/reverse_circuit.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/utils/permutation.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

boost::dynamic_bitset<> get_optimization_vector( const std::string& methods )
{
  boost::dynamic_bitset<> v( 5u );

  for ( auto c : methods )
  {
    switch ( c )
    {
    case 'm': v.set( 0u ); break;
    case 'n': v.set( 1u ); break;
    case 'a': v.set( 2u ); break;
    case 'e': v.set( 3u ); break;
    case 's': v.set( 4u ); break;
    }
  }

  return v;
}

boost::dynamic_bitset<> control_mask( const gate::control_container& controls, unsigned n )
{
  boost::dynamic_bitset<> cs( n );

  for ( const auto& c : controls )
  {
    assert( c.line() < n );

    cs[c.line()] = 1;
  }

  return cs;
}

std::pair<boost::dynamic_bitset<>, boost::dynamic_bitset<>> control_polarity_mask( const gate::control_container& controls, unsigned n )
{
  boost::dynamic_bitset<> cs( n ), ps( n );

  for ( const auto& c : controls )
  {
    assert( c.line() < n );

    cs[c.line()] = 1;
    ps[c.line()] = c.polarity();
  }

  return {cs, ps};
}

gate::control_container make_controls( const boost::dynamic_bitset<>& cs, const boost::dynamic_bitset<>& ps )
{
  gate::control_container controls;

  foreach_bit( cs, [&controls, &ps]( unsigned pos ) {
      controls.push_back( make_var( pos, ps[pos] ) );
    } );

  return controls;
}

circuit simplify_not_gates( const circuit& base )
{
  circuit circ;
  circ.set_lines( base.lines() );
  copy_metadata( base, circ );

  boost::dynamic_bitset<> not_state( circ.lines() );

  //std::cout << "[i] NOT state:" << not_state << std::endl;

  for ( auto i = 0u; i < base.num_gates(); ++i )
  {
    const auto& g = base[i];

    assert( is_toffoli( g ) );
    const auto target = g.targets().front();

    if ( g.controls().empty() )
    {
      //std::cout << "[i] found NOT gate at pos " << i << " using target " << target << std::endl;
      /* do not copy but remember */
      not_state.flip( target );
    }
    else if ( g.controls().size() == 1 && not_state[target] )
    {
      const auto& ctrl = g.controls().front();
      //std::cout << "[i] merge NOT gate at pos " << i << " using target " << target << std::endl;
      /* merge stored not gate into CNOT with the same target (we may have a target at the control line) */
      append_cnot( circ, make_var( ctrl.line(), !ctrl.polarity() != not_state[ctrl.line()] ), target );
      not_state.reset( target );
    }
    else
    {
      /* flip controls based on remembered NOT gates */
      gate::control_container controls;

      for ( const auto& c : g.controls() )
      {
        controls.push_back( make_var( c.line(), c.polarity() != not_state[c.line()] ) );
      }

      append_toffoli( circ, controls, target );
    }
  }

  //std::cout << "[i] NOT state (remaining):" << not_state << std::endl;

  /* remaining NOT gates */
  foreach_bit( not_state, [&circ]( unsigned pos ) {
      append_not( circ, pos );
    } );

  return circ;
}

circuit simplify_adjacent( const circuit& base )
{
  circuit circ;
  circ.set_lines( base.lines() );
  copy_metadata( base, circ );

  auto pos = 0u;

  while ( pos < base.num_gates() )
  {
    if ( pos + 1 == base.num_gates() )
    {
      /* last gate */
      circ.append_gate() = base[pos];
      break;
    }

    const auto& g1 = base[pos];
    const auto& g2 = base[pos + 1];

    /* candidate for simplication */
    if ( is_toffoli( g1 ) && is_toffoli( g2 ) && ( g1.targets().front() == g2.targets().front() ) && abs( static_cast<int>( g1.controls().size() ) - static_cast<int>( g2.controls().size() ) ) <= 1 )
    {
      boost::dynamic_bitset<> cs1, ps1, cs2, ps2;

      std::tie( cs1, ps1 ) = control_polarity_mask( g1.controls(), base.lines() );
      std::tie( cs2, ps2 ) = control_polarity_mask( g2.controls(), base.lines() );

      const auto cdc = ( cs1 ^ cs2 ).count();
      const auto pdc = ( ps1 ^ ps2 ).count();

      if ( cs1 == cs2 && ps1 == ps2 )
      {
        /* same gate, remove */
        pos += 2;
      }
      else if ( cs1 == cs2 && pdc == 1 )
      {
        /* remove one control */
        const auto bit = ( ps1 ^ ps2 ).find_first();
        assert( cs1[bit] );
        cs1[bit] = 0;

        append_toffoli( circ, make_controls( cs1, ps1 ), g1.targets().front() );

        pos += 2;
      }
      else if ( cdc == 1 && pdc <= 1 && ( ( pdc == 0 ) || ( cs1 ^ cs2 ).find_first() == ( ps1 ^ ps2 ).find_first() ) )
      {
        const auto bit = ( cs1 ^ cs2 ).find_first();
        cs1[bit] = true;
        ps1[bit] = pdc == 1 ? false : true;

        append_toffoli( circ, make_controls( cs1, ps1 ), g1.targets().front() );

        pos += 2;
      }
      else
      {
        circ.append_gate() = base[pos++];
      }
    }
    else
    {
      circ.append_gate() = base[pos++];
    }
  }

  return circ;
}

circuit simplify_swap_gates( const circuit& base, std::vector<unsigned>& perm )
{
  circuit circ;
  circ.set_lines( base.lines() );
  copy_metadata( base, circ );

  boost::iota( perm, 0u );

  std::vector<std::pair<unsigned, unsigned>> prefix;
  const auto add_prefix = [&prefix, &circ, &perm]() {
    if ( prefix.size() == 2u )
    {
      const auto control = prefix.front().first;
      const auto target = prefix.front().second;
      append_cnot( circ, perm[target], perm[control] ); /* already swap this one */
      std::swap( perm[control], perm[target] );
    }
    else
    {
      assert( prefix.size() <= 1u );
      for ( const auto& gp : prefix )
      {
        append_cnot( circ, perm[gp.first], perm[gp.second] );
      }
    }
    prefix.clear();
  };

  for ( auto i = 0u; i < base.num_gates(); ++i )
  {
    const auto& g = base[i];
    if ( !is_toffoli( g ) || g.controls().size() != 1u || !g.controls().front().polarity() )
    {
      add_prefix();

      auto& new_gate = circ.append_gate();
      new_gate.set_type( g.type() );
      for ( const auto& c : g.controls() )
      {
        new_gate.add_control( make_var( perm[c.line()], c.polarity() ) );
      }
      for ( const auto& t : g.targets() )
      {
        new_gate.add_target( perm[t] );
      }
      continue;
    }

    /* interesting case fsm */
    const auto control = g.controls().front().line();
    const auto target = g.targets().front();

    //std::cout << "[i] found CNOT gate at pos " << i << std::endl;

    switch ( prefix.size() )
    {
    case 0u:
      //std::cout << "[i]  push first on stack" << std::endl;
      prefix.push_back( {control, target} );
      break;

    case 1u:
      if ( prefix.front().first == target && prefix.front().second == control )
      {
        //std::cout << "[i]  push second on stack" << std::endl;
        prefix.push_back( {control, target} );
      }
      else
      {
        //std::cout << "[i]  no match after 1, apply prefix" << std::endl;
        add_prefix();
        append_cnot( circ, perm[control], perm[target] );
      }
      break;

    case 2u:
      if ( prefix.front().first == control && prefix.front().second == target )
      {
        //std::cout << "[i]  match, reset prefix" << std::endl;
        prefix.clear();
        std::swap( perm[control], perm[target] );
      }
      else
      {
        //std::cout << "[i]  no match after 2, apply prefix" << std::endl;
        add_prefix();
        append_cnot( circ, perm[control], perm[target] );
      }
      break;
    }
  }

  add_prefix();

  return circ;
}

/* tries to move gates with same target line together, only forward looking moving backwards */
circuit simple_merge_heuristic( const circuit& base )
{
  circuit circ( base.lines() );
  copy_metadata( base, circ );

  boost::dynamic_bitset<> moved( base.num_gates() );

  for ( auto i = 0u; i < base.num_gates(); ++i )
  {
    /* already moved */
    if ( moved[i] ) { continue; }

    /* copy gate at pos i */
    circ.append_gate() = base[i];

    /* merge only Toffoli gates */
    if ( !is_toffoli( base[i] ) ) { continue; }

    auto target = base[i].targets().front();
    boost::dynamic_bitset<> block( base.lines() );

    /* try to move other gates */
    for ( auto j = i + 1; j < base.num_gates(); ++j )
    {
      if ( moved[j] ) { continue; }
      if ( !is_toffoli( base[j] ) ) { break; }

      const auto mask = control_mask( base[j].controls(), base.lines() );
      const auto target_j = base[j].targets().front();

      /* control blocks target of gate i */
      if ( mask[target] )
      {
        break;
      }
      /* same target and valid move */
      else if ( target_j == target && ( block & mask ).none() )
      {
        circ.append_gate() = base[j];
        moved.set( j );
      }
      else
      {
        block.set( target_j );
      }
    }
  }

  return circ;
}

/* moves target-adjacent gates using recomputing control lines with exorcism */
void exorcism_merge_heuristic_add_cubes( circuit& circ, const cube_vec_t& cubes, int current_target )
{
  if ( cubes.empty() ) { return; }

  /* just one gate, directly add it */
  if ( cubes.size() == 1u )
  {
    append_toffoli( circ, make_controls( cubes.front().care(), cubes.front().bits() ), current_target );
    return;
  }

  /* we call exorcism */
  auto f = [&circ, current_target]( const cube_t& c ) {
    /* c.first is bits, c.second is care */
    append_toffoli( circ, make_controls( c.second, c.first ), current_target );
  };
  auto settings = std::make_shared<properties>();
  settings->set( "on_cube", cube_function_t( f ) );
  settings->set( "verbose", false );
  exorcism_minimization( cubes, settings );
}

circuit exorcism_merge_heuristic( const circuit& base )
{
  circuit circ;
  circ.set_lines( base.lines() );
  copy_metadata( base, circ );

  auto current_target = -1;
  cube_vec_t cubes;

  for ( const auto& gate : base )
  {
    /* works only for Toffoli gates for now */
    if ( !is_toffoli( gate ) )
    {
      exorcism_merge_heuristic_add_cubes( circ, cubes, current_target );
      cubes.clear();
      current_target = -1;
      circ.append_gate() = gate;
      continue;
    }

    const auto target = gate.targets().front();

    /* apply current cubes */
    if ( static_cast<int>( target ) != current_target )
    {
      exorcism_merge_heuristic_add_cubes( circ, cubes, current_target );
      cubes.clear();
      current_target = target;
    }

    /* add new cube */
    const auto mask = control_polarity_mask( gate.controls(), circ.lines() );
    cubes.push_back( cube( mask.second, mask.first ) );
  }

  /* leftovers? */
  exorcism_merge_heuristic_add_cubes( circ, cubes, current_target );

  return circ;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

bool simplify( circuit& circ, const circuit& base, properties::ptr settings, properties::ptr statistics )
{
  /* settings */
  const auto methods     = get( settings, "methods",     std::string( "mnaes" ) );
  const auto reverse_opt = get( settings, "reverse_opt", true );
  const auto verbose     = get( settings, "verbose",     false );

  /* timer */
  properties_timer t( statistics );

  circuit tmp;
  copy_circuit( base, tmp );

  /* for verbose output */
  const auto vsize_out = [verbose, &tmp]( const std::string& method ) {
    if ( verbose )
    {
      std::cout << boost::format( "[i] %20s \033[1;32msize: %d\033[0m" ) % method % tmp.num_gates() << std::endl;
    }
  };

  const auto methods_vec = get_optimization_vector( methods );
  auto improvement = true;
  auto round = 1u;
  std::vector<unsigned> perm( base.lines() ), gperm( base.lines() );
  boost::iota( gperm, 0u );
  while ( improvement )
  {
    auto size = tmp.num_gates();

    vsize_out( boost::str( boost::format( "\033[1;31moptimization round %d\033[0m" ) % round ) );

    if ( methods_vec[0u] ) { tmp = simple_merge_heuristic( tmp );   vsize_out( "simple merge" ); }
    if ( methods_vec[1u] ) { tmp = simplify_not_gates( tmp );       vsize_out( "not gates" ); }
    if ( methods_vec[2u] ) { tmp = simplify_adjacent( tmp );        vsize_out( "adjacent" ); }
    if ( methods_vec[3u] ) { tmp = exorcism_merge_heuristic( tmp ); vsize_out( "exorcism" ); }
    if ( methods_vec[4u] ) {
      tmp = simplify_swap_gates( tmp, perm ); vsize_out( "swap" );
      gperm = permutation_multiply( gperm, perm );
    }

    if ( reverse_opt )
    {
      reverse_circuit( tmp );
      if ( methods_vec[0u] ) { tmp = simple_merge_heuristic( tmp );   vsize_out( "simple merge (r)" ); }
      if ( methods_vec[1u] ) { tmp = simplify_not_gates( tmp );       vsize_out( "not gates (r)" ); }
      if ( methods_vec[2u] ) { tmp = simplify_adjacent( tmp );        vsize_out( "adjacent (r)" ); }
      if ( methods_vec[3u] ) { tmp = exorcism_merge_heuristic( tmp ); vsize_out( "exorcism (r)" ); }
      if ( methods_vec[4u] ) {
        //tmp = simplify_swap_gates( tmp, perm ); vsize_out( "swap (r)" );
      }
      reverse_circuit( tmp );
    }

    improvement = size > tmp.num_gates();

    ++round;
  }

  copy_circuit( tmp, circ );
  copy_metadata( base, circ );

  std::vector<std::string> outputs( circ.lines() );
  for ( auto i = 0u; i < circ.lines(); ++i )
  {
    outputs[i] = circ.outputs()[gperm[i]];
  }
  circ.set_outputs( outputs );

  return true;
}

optimization_func simplify( properties::ptr settings, properties::ptr statistics )
{
  optimization_func f = [&settings, &statistics]( circuit& circ, const circuit& base ) {
    return simplify( circ, base, settings, statistics );
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
