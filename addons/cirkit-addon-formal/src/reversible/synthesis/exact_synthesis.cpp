/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2011  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include "exact_synthesis.hpp"

#include <cmath>
#include <fstream>
#include <iostream>

#include <boost/assign/std/vector.hpp>
#include <boost/assign/std/set.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/utils/timer.hpp>

#include <reversible/functions/fully_specified.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/copy_metadata.hpp>

#include <z3++.h>

namespace cirkit
{
  using namespace boost::assign;

  z3::expr operator<<(z3::expr const & a, z3::expr const & b) {
    check_context(a, b);
    assert(a.is_bv() && b.is_bv());
    Z3_ast r = Z3_mk_bvshl(a.ctx(), a, b);
    a.check_error();
    return z3::expr(a.ctx(), r);
  }

  z3::expr ite(z3::expr const & a, z3::expr const & b, z3::expr const & c) {
    check_context(a, b);
    check_context(b, c);
    assert(a.is_bool());
    Z3_ast r = Z3_mk_ite(a.ctx(), a, b, c);
    a.check_error();
    return z3::expr(a.ctx(), r);
  }

  boost::dynamic_bitset<> to_bitset( const z3::expr& a ) {
    std::stringstream s;
    s << a;
    return boost::dynamic_bitset<>( s.str().substr( 2u ) );
  }

  bool synthesis_for_fixed_length( circuit& circ, const binary_truth_table& spec, unsigned k )
  {
    using boost::str;
    using boost::format;

    unsigned n = spec.num_inputs();
    unsigned rows = std::distance( spec.begin(), spec.end() );

    // Context and solver
    z3::context ctx;
    z3::solver solver( ctx );

    // Variables
    std::vector<std::pair<z3::expr, z3::expr> > network;
    std::vector<std::vector<z3::expr> > gate_values;

    for ( int i : boost::irange( 0u, k + 1u) ) {
      gate_values += std::vector<z3::expr>();
      for ( int j : boost::irange( 0u, rows ) ) {
        gate_values[i] += ctx.bv_const( str( format( "gate-value-%d-%d" ) % i % j ).c_str(), n );
      }
    }

    // Constraints for gate operations
    z3::expr zero = ctx.bv_val( 0, n );
    z3::expr one  = ctx.bv_val( 1, n );
    for ( int i : boost::irange( 0u, k ) ) {
      z3::expr control = ctx.bv_const( str( format( "control-%d" ) % i ).c_str(), n );
      z3::expr target  = ctx.bv_const( str( format( "target-%d" ) % i ).c_str(),  n );

      network += std::make_pair( control, target );

      solver.add( ( control | (one << target ) ) != control );
      solver.add( z3::ule( target, ctx.bv_val( n, n ) ) );

      for ( unsigned j = 0u; j < rows; ++j )
      {
        z3::expr hit = ctx.bool_const( str( format( "hit-%d-%d" ) % i % j ).c_str() );
        solver.add( hit == ((gate_values[i][j] & control) == control));
        solver.add( gate_values[i + 1][j] == (gate_values[i][j] ^ (ite(hit, one, zero) << target)));
      }
    }

    // Constraints for inputs and outputs
    for ( binary_truth_table::const_iterator iter = spec.begin(); iter != spec.end(); ++iter ) {
      boost::dynamic_bitset<> in( n );
      boost::dynamic_bitset<> mask( n );
      boost::dynamic_bitset<> out( n );

      unsigned bitpos = 0;
      for ( const auto& bit : boost::make_iterator_range( iter->first ) ) {
        in.set( bitpos++, *bit );
      }

      bitpos = 0;
      for ( const auto& bit : boost::make_iterator_range( iter->second ) ) {
        mask.set( bitpos, bit );
        out.set( bitpos++, bit && *bit );
      }
      unsigned pos = std::distance ( spec.begin(), iter );
      solver.add( gate_values[0][pos] == ctx.bv_val( (__uint64)in.to_ulong(), n ) );
      solver.add( ( gate_values[k][pos] & ctx.bv_val( (__uint64)mask.to_ulong(), n ) ) == ctx.bv_val( (__uint64)out.to_ulong(), n ) );
    }

    if ( solver.check() == z3::sat ) {
      z3::model m = solver.get_model();

      for ( unsigned i : boost::irange( 0u, k ) ) {
        auto eval_control = to_bitset( m.eval( network[i].first ) );
        auto eval_target = to_bitset( m.eval( network[i].second ) ).to_ulong();

        gate::control_container controls;
        for ( unsigned j : boost::irange( 0u, n ) ) {
          if ( eval_control.test( j ) ) {
            controls += make_var( j );
          }
        }
        append_toffoli( circ, controls, eval_target );
      }

      return true;
    } else {
      return false;
    }
  }

  bool exact_synthesis( circuit& circ, const binary_truth_table& spec, properties::ptr settings, properties::ptr statistics )
  {
    unsigned max_depth = get<unsigned>( settings, "max_depth", 20u );

    timer<properties_timer> t;

    if ( statistics )
    {
      properties_timer rt( statistics );
      t.start( rt );
    }

    circ.set_lines( spec.num_inputs() );

    unsigned k = 0u;
    bool result = false;
    do {
      std::cout << "Check for depth " << k << std::endl;
      result = synthesis_for_fixed_length( circ, spec, k );
    } while ( !result && k++ < max_depth );

    if (result) {
      copy_metadata ( spec, circ );
    }
    else {
      set_error_message (statistics, "Could not find a circuit within the predefined depth.");
    }

    return result;
  }

  truth_table_synthesis_func exact_synthesis_func ( properties::ptr settings, properties::ptr statistics )
  {
    truth_table_synthesis_func f = [&settings, &statistics]( circuit& circ, const binary_truth_table& spec ) {
      return exact_synthesis( circ, spec, settings, statistics );
    };
    f.init ( settings, statistics );
    return f;
  }

}

// Local Variables:
// c-basic-offset: 2
// End:
