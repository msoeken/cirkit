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

#include "exact_synthesis.hpp"

#include <cmath>
#include <fstream>
#include <iostream>
#include <tuple>

#include <boost/assign/std/vector.hpp>
#include <boost/assign/std/set.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/utils/timer.hpp>
#include <formal/utils/z3_utils.hpp>

#include <reversible/functions/fully_specified.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/copy_metadata.hpp>

#include <z3++.h>

namespace cirkit
{
using namespace boost::assign;

using gate_constraint_fun = std::function<void( z3::context& ctx, z3::solver& solver,
                                                circuit&, std::vector<std::vector<z3::expr>>& network,
                                                std::vector<std::vector<z3::expr> >& gate_values, const binary_truth_table&,
                                                unsigned k, unsigned n )>;

using evaluation_fun = std::function<bool( z3::solver& solver, circuit& circ,
                                           std::vector<std::vector<z3::expr>>& network, unsigned k, unsigned n )>;

using symmetry_fun = std::function<void( z3::context&, z3::solver&, std::vector<std::vector<z3::expr>>&, unsigned)>;

void gate_constraints_original( z3::context& ctx, z3::solver& solver, circuit&,
                                std::vector<std::vector<z3::expr> >& network,
                                std::vector<std::vector<z3::expr> >& gate_values,
                                const binary_truth_table& spec, unsigned k, unsigned n )
{
  using boost::str;
  using boost::format;

  auto rows = boost::distance( spec );

  auto zero = ctx.bv_val(0, n);
  auto one = ctx.bv_val(1, n);

  for ( auto i = 0u; i < k; ++i )
  {
    auto control = ctx.bv_const(str(format("control-%d") % i).c_str(), n);
    auto target = ctx.bv_const(str(format("target-%d") % i).c_str(), n);

    network += std::vector<z3::expr>{ control, target };

    solver.add( (control | (one << target)) != control );
    solver.add( z3::ule(target, ctx.bv_val(n, n)) );

    for ( auto j = 0u; j < rows; ++j )
    {
      auto hit = ctx.bool_const(str(format("hit-%d-%d") % i % j).c_str());
      solver.add( hit == ((gate_values[i][j] & control) == control) );
      solver.add(
          gate_values[i + 1][j]
          == (gate_values[i][j] ^ (ite(hit, one, zero) << target)));
    }
  }
}

void gate_constraints_negative_control( z3::context& ctx, z3::solver& solver,
                                        circuit&, std::vector<std::vector<z3::expr> >& network,
                                        std::vector<std::vector<z3::expr> >& gate_values,
                                        const binary_truth_table& spec, unsigned k, unsigned n )
{
  using boost::str;
  using boost::format;

  auto rows = boost::distance( spec );

  auto zero = ctx.bv_val(0, n);
  auto one = ctx.bv_val(1, n);
  for ( auto i = 0u; i < k; ++i )
  {
    auto control = ctx.bv_const(str(format("control-%d") % i).c_str(), n);
    auto target = ctx.bv_const(str(format("target-%d") % i).c_str(), n);
    auto polarity = ctx.bv_const(str(format("polarity-%d") % i).c_str(), n);

    network += std::vector<z3::expr>{ control, target, polarity };

    solver.add( (control | (one << target)) != control );
    solver.add( z3::ule(target, ctx.bv_val(n, n)) );

    for ( auto j = 0u; j < rows; ++j )
    {
      auto hit = ctx.bool_const(str(format("hit-%d-%d") % i % j).c_str());
      solver.add(
          hit == (((gate_values[i][j] ^ polarity) & control) == control));
      solver.add(
          gate_values[i + 1][j]
          == (gate_values[i][j] ^ (ite(hit, one, zero) << target)));
    }
  }
}

void gate_constraints_negative_control_multiple_target( z3::context& ctx,
                                                        z3::solver& solver, circuit&, std::vector<std::vector<z3::expr> >& gates,
                                                        std::vector<std::vector<z3::expr> >& line_values,
                                                        const binary_truth_table& spec, unsigned k, unsigned n )
{

  using boost::str;
  using boost::format;

  auto rows = boost::distance( spec );

  auto zero = ctx.bv_val(0, n);
  auto one = ctx.bv_val(1, n);
  for ( auto i = 0u; i < k; ++i )
  {
    auto control = ctx.bv_const(str(format("control-%d") % i).c_str(), n);
    auto target = ctx.bv_const(str(format("target-%d") % i).c_str(), n);
    auto polarity = ctx.bv_const(str(format("polarity-%d") % i).c_str(), n);

    gates += std::vector<z3::expr>{ control, target, polarity };

    solver.add((control & target) == zero);

    for ( auto j = 0u; j < rows; ++j )
    {
      auto hit = ctx.bool_const(str(format("hit-%d-%d") % i % j).c_str());
      solver.add(
          hit == (((line_values[i][j] ^ polarity) & control) == control));
      solver.add(
          line_values[i + 1][j]
          == ite(hit, line_values[i][j] ^ target, line_values[i][j]));
    }
  }
}

void gate_constraints_multiple_target( z3::context& ctx, z3::solver& solver,
                                       circuit&, std::vector<std::vector<z3::expr> >& network,
                                       std::vector<std::vector<z3::expr> >& gate_values,
                                       const binary_truth_table& spec, unsigned k, unsigned n )
{
  using boost::str;
  using boost::format;

  auto rows = boost::distance( spec );

  auto zero = ctx.bv_val(0, n);
  auto one = ctx.bv_val(1, n);

  for ( auto i = 0u; i < k; ++i )
  {
    auto control = ctx.bv_const(str(format("control-%d") % i).c_str(), n);
    auto target = ctx.bv_const(str(format("target-%d") % i).c_str(), n);

    network += std::vector<z3::expr>{ control, target };

    solver.add((control & target) == zero);

    for ( auto j = 0u; j < rows; ++j )
    {
      auto hit = ctx.bool_const(str(format("hit-%d-%d") % i % j).c_str());
      solver.add(hit == ((gate_values[i][j] & control) == control));
      solver.add(
          gate_values[i + 1][j]
          == ite(hit, gate_values[i][j] ^ target, gate_values[i][j]));
    }
  }
}

bool eval_original(z3::solver& solver, circuit& circ,
    std::vector<std::vector<z3::expr>>& network, unsigned k, unsigned n)
{
  if (solver.check() == z3::sat)
  {
    z3::model m = solver.get_model();
    for (unsigned i : boost::irange(0u, k))
    {
      auto eval_control = to_bitset(m.eval(network[i][0]));
      auto eval_target = to_bitset(m.eval(network[i][1])).to_ulong();

      gate::control_container controls;
      for (unsigned j : boost::irange(0u, n))
      {
        if (eval_control.test(j))
        {
          controls += make_var(j);
        }
      }

      append_toffoli(circ, controls, eval_target);
    }
    return true;
  } else
  {
    return false;
  }
}

bool eval_multiple_target(z3::solver& solver, circuit& circ,
    std::vector<std::vector<z3::expr>>& gates, unsigned k, unsigned n)
{
  if (solver.check() == z3::sat)
  {
    z3::model m = solver.get_model();

    for (unsigned i : boost::irange(0u, k))
    {
      auto eval_control = to_bitset(m.eval(gates[i][0]));
      auto eval_target = to_bitset(m.eval(gates[i][1]));

      gate::control_container controls;
      for (unsigned j : boost::irange(0u, n))
      {
        if (eval_control.test(j))
        {
          controls += make_var(j);
        }
      }

      gate::target_container targets;
      for (unsigned j : boost::irange(0u, n))
      {
        if (eval_target.test(j))
        {
          targets += j;
        }
      }

      for ( const auto& target : targets )
      {
        append_toffoli(circ, controls, target);
      }
    }

    return true;
  } else
  {
    return false;
  }
}

bool eval_negative_control(z3::solver& solver, circuit& circ,
    std::vector<std::vector<z3::expr>>& gates, unsigned k, unsigned n)
{
  if (solver.check() == z3::sat)
  {
    z3::model m = solver.get_model();

    for (unsigned i : boost::irange(0u, k))
    {
      auto eval_control = to_bitset(m.eval(gates[i][0]));
      auto eval_target = to_bitset(m.eval(gates[i][1])).to_ulong();
      auto eval_polarity = to_bitset(m.eval(gates[i][2]));

      gate::control_container controls;
      for (unsigned j : boost::irange(0u, n))
      {
        if (eval_control.test(j))
        {
          controls += make_var(j, !eval_polarity.test(j));
        }
      }

      append_toffoli(circ, controls, eval_target);
    }

    return true;
  } else
  {
    return false;
  }

}

bool eval_negative_control_multiple_target(z3::solver& solver, circuit& circ,
    std::vector<std::vector<z3::expr>>& gates, unsigned k, unsigned n)
{
  if (solver.check() == z3::sat)
  {
    z3::model m = solver.get_model();

    for (unsigned i : boost::irange(0u, k))
    {
      auto eval_control = to_bitset(m.eval(gates[i][0]));
      auto eval_polarity = to_bitset(m.eval(gates[i][2]));
      auto eval_target = to_bitset(m.eval(gates[i][1]));

//      std::cout << "control: " << eval_control << "\n";
//      std::cout << "polarity: " << eval_polarity << "\n";
//      std::cout << "target: " << eval_target << "\n";


      gate::control_container controls;
      for (unsigned j : boost::irange(0u, n))
      {
        if (eval_control.test(j))
        {
          controls += make_var(j, !eval_polarity.test(j));
        }
      }

      gate::target_container targets;
      for (unsigned j : boost::irange(0u, n))
      {
        if (eval_target.test(j))
        {
//          std::cout <<"j: " <<  j << "\n";
          targets += j;
        }
      }

      for ( const auto& target : targets )
      {
        append_toffoli(circ, controls, target);
      }
    }

    return true;
  } else
  {
    return false;
  }
}

void input_output_constraints(z3::context& ctx, z3::solver solver,
    const binary_truth_table& spec,
    std::vector<std::vector<z3::expr> >& gate_values, unsigned k, unsigned n)
{
// Constraints for inputs and outputs. This is
  for (binary_truth_table::const_iterator iter = spec.begin();
      iter != spec.end(); ++iter)
  {
    boost::dynamic_bitset<> in(n);
    boost::dynamic_bitset<> mask(n);
    boost::dynamic_bitset<> out(n);

    unsigned bitpos = 0;
    for (const auto& bit : boost::make_iterator_range(iter->first))
    {
      in.set(bitpos++, *bit);
    }

    bitpos = 0;
    for (const auto& bit : boost::make_iterator_range(iter->second))
    {
      mask.set(bitpos, (bool)bit);
      out.set(bitpos++, (bool)bit && *bit);
    }
    unsigned pos = std::distance<binary_truth_table::const_iterator>(
        spec.begin(), iter);
    solver.add(gate_values[0][pos] == ctx.bv_val((__uint64) in.to_ulong(), n));
    solver.add(
        (gate_values[k][pos] & ctx.bv_val((__uint64) mask.to_ulong(), n))
            == ctx.bv_val((__uint64) out.to_ulong(), n));
  }
}

/******************************************************************************
 * symmetry breaking                                                          *
 ******************************************************************************/

void symmetry_breaking_no_double_gate( z3::context& ctx, z3::solver& solver, const std::vector<std::vector<z3::expr>>& network, unsigned n )
{
  if ( network.size() >= 2 )
  {
    for ( auto i = 0u; i < network.size() - 1; ++i )
    {
      solver.add( !( ( network[i][0] == network[i + 1][0] ) && ( network[i][1] == network[i + 1][1] ) ) );
    }
  }
}

void symmetry_breaking_colexicographic( z3::context& ctx, z3::solver& solver, const std::vector<std::vector<z3::expr>>& network, unsigned n )
{
  const auto one = ctx.bv_val( 1u, n );
  const auto zero = ctx.bv_val( 0u, n );

  if ( network.size() >= 2 )
  {
    for ( auto i = 0u; i < network.size() - 1; ++i )
    {
      const z3::expr move1 = ( ( one << network[i][0] ) & network[i + 1][1] ) == zero;
      const z3::expr move2 = ( network[i][1] & ( one << network[i + 1][0] ) ) == zero;

      const z3::expr cless = z3::ule( network[i][0], network[i + 1][0] );
      const z3::expr ceq   = ( network[i][0] == network[i + 1][0] ) && ( z3::ule( network[i][1], network[i + 1][1] ) );

      solver.add( implies( move1 && move2, cless || ceq ) );
    }
  }
}

void symmetry_breaking_one_not_per_line( z3::context& ctx, z3::solver& solver, const std::vector<std::vector<z3::expr>>& network, unsigned n )
{
  const auto zero = ctx.bv_val( 0u, n );

  for ( auto i = 0u; i < network.size(); ++i )
  {
    for ( auto j = i + 1; j < network.size(); ++j )
    {
      /* if some gate i has no controls (i.e., NOT), then there can't be another on that line */
      solver.add( implies( network[i][0] == zero && network[i][1] == network[j][1], network[j][0] != zero ) );
    }
  }
}

bool synth_len(circuit& circ, const binary_truth_table& spec,
               gate_constraint_fun gate_constraints,
               evaluation_fun eval, const std::vector<symmetry_fun>& symmetry_breaking, bool only_toffoli, unsigned gate_count)
{
  using boost::str;
  using boost::format;

  unsigned n = spec.num_inputs();
  unsigned rows = std::distance(spec.begin(), spec.end());

  z3::context ctx;
  z3::solver solver(ctx);

  // Instead of having vector<pair<exp,exp>> we use vector<vector<exp>>. This
  // allows for more flexibility. The inner vector should have a length of
  // 2 or 3.
  // In case of len 2, the first entry contains the controls, the second the targets
  // In case of len 3, the last entry contains the polarity of the controls
  std::vector<std::vector<z3::expr> > network;

  // this vector of vectores stores, for every input pattern (outer vector index), the
  // line values after the k'th gate (inner vector index)
  std::vector<std::vector<z3::expr> > gate_values;

  // initialize the gate values
  for (int i : boost::irange(0u, gate_count + 1u))
  {
    gate_values += std::vector<z3::expr>();
    for (int j : boost::irange(0u, rows))
    {
      gate_values[i] += ctx.bv_const(
          str(format("gate-value-%d-%d") % i % j).c_str(), n);
    }
  }

  input_output_constraints(ctx, solver, spec, gate_values, gate_count, n);

  gate_constraints(ctx, solver, circ, network, gate_values, spec, gate_count,
      n);

  for ( const auto& f : symmetry_breaking )
  {
    f( ctx, solver, network, n );
  }

  if ( only_toffoli )
  {
    for ( auto i = 0u; i < gate_count; ++i )
    {
      auto constr = ( network[i][0] == ctx.bv_val( 0u, n ) );
      for ( auto j = 0u; j < n; ++j )
      {
        auto mask = 1 << j;
        constr = constr || ( network[i][0] == ctx.bv_val( mask, n ) );

        for ( auto k = j + 1; k < n; ++k )
        {
          constr = constr || ( network[i][0] == ctx.bv_val( mask | ( 1 << k ), n ) );
        }
      }
      solver.add( constr );
    }
  }

  return eval(solver, circ, network, gate_count, n);
}

/******************************************************************************
 * public functions                                                           *
 ******************************************************************************/


bool exact_synthesis(circuit& circ, const binary_truth_table& spec,
    properties::ptr settings, properties::ptr statistics)
{
  const auto start_depth  = get( settings, "start_depth",  0u );
  const auto max_depth    = get( settings, "max_depth",    20u );
  const auto negative     = get( settings, "negative",     false );
  const auto multiple     = get( settings, "multiple",     false );
  const auto only_toffoli = get( settings, "only_toffoli", false );
  const auto verbose      = get( settings, "verbose",      false );

  properties_timer t( statistics );

  circ.set_lines(spec.num_inputs());

  unsigned gate_count = start_depth;
  bool result = false;

  gate_constraint_fun gate_constraints = &gate_constraints_original;
  evaluation_fun eval = &eval_original;
  std::vector<symmetry_fun> symmetry_breaking;

  symmetry_breaking.push_back( symmetry_breaking_no_double_gate );
  if (negative && multiple)
  {
    gate_constraints = &gate_constraints_negative_control_multiple_target;
    eval = &eval_negative_control_multiple_target;
    symmetry_breaking.push_back( symmetry_breaking_one_not_per_line );
  }
  else
  {
    //symmetry_breaking.push_back( symmetry_breaking_colexicographic );
    if (negative)
    {
      gate_constraints = &gate_constraints_negative_control;
      eval = &eval_negative_control;
      symmetry_breaking.push_back( symmetry_breaking_one_not_per_line );
    }
    if (multiple)
    {
      gate_constraints = &gate_constraints_multiple_target;
      eval = &eval_multiple_target;
    }
  }

  do
  {
    if ( verbose )
    {
      std::cout << "[i] check for depth " << gate_count << std::endl;
    }
    result = synth_len( circ, spec, gate_constraints, eval, symmetry_breaking, only_toffoli, gate_count );
  } while (!result && gate_count++ < max_depth);

  if (result)
  {
    copy_metadata(spec, circ);
  } else
  {
    set_error_message(statistics,
        "Could not find a circuit within the predefined depth.");
  }

  return result;
}

truth_table_synthesis_func exact_synthesis_func(properties::ptr settings,
    properties::ptr statistics)
{
  truth_table_synthesis_func f =
      [&settings, &statistics]( circuit& circ, const binary_truth_table& spec )
      {
        return exact_synthesis( circ, spec, settings, statistics );
      };
  f.init(settings, statistics);
  return f;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
