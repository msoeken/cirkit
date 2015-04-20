/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

#if ADDON_FORMAL

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

bool synth_len(circuit& circ, const binary_truth_table& spec,
    properties::ptr settings, gate_constraint_fun gate_constraints,
    evaluation_fun eval, unsigned gate_count)
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

  return eval(solver, circ, network, gate_count, n);

}

bool exact_synthesis(circuit& circ, const binary_truth_table& spec,
    properties::ptr settings, properties::ptr statistics)
{
  unsigned max_depth = get<unsigned>( settings, "max_depth", 20u );
  bool     negative  = get<bool>(     settings, "negative",  false );
  bool     multiple  = get<bool>(     settings, "multiple",  false );
  bool     verbose   = get<bool>(     settings, "verbose",   false );

  properties_timer t( statistics );

  circ.set_lines(spec.num_inputs());

  unsigned gate_count = 0u;
  bool result = false;

  gate_constraint_fun gate_constraints = &gate_constraints_original;
  evaluation_fun eval = &eval_original;

  if (negative && multiple)
  {
    gate_constraints = &gate_constraints_negative_control_multiple_target;
    eval = &eval_negative_control_multiple_target;
  } else
  {
    if (negative)
    {
      gate_constraints = &gate_constraints_negative_control;
      eval = &eval_negative_control;
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
      std::cout << "[I] check for depth " << gate_count << std::endl;
    }
    result = synth_len(circ, spec, settings, gate_constraints, eval,
        gate_count);
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

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
