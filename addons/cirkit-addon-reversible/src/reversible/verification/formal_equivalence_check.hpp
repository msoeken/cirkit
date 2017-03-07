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

#ifndef FORMAL_VERIFICATION_HPP
#define FORMAL_VERIFICATION_HPP

#if defined(HAS_METASMT)

#include <reversible/circuit.hpp>
#include <reversible/target_tags.hpp>

#include <metaSMT/DirectSolver_Context.hpp>
#include <metaSMT/frontend/Logic.hpp>

#include <boost/dynamic_bitset.hpp>
#include <boost/range/counting_range.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/timer.hpp>

namespace cirkit {

class ec_check_result
{
  public:

  private:
//    std::vector < boost::dynamic_bitset> >
};

std::vector < metaSMT::logic::predicate > allocate_inputs ( unsigned inputs );

template<typename Solver>
std::vector < metaSMT::logic::predicate > get_control_predicates (
    Solver& solver
  , gate::control_container const& controls
  , std::vector < metaSMT::logic::predicate > const& vars
) {
  using namespace metaSMT::logic;

  // check that all controls are positive polarity
  bool positive = std::all_of (
        controls.begin()
      , controls.end()
      , [] ( const variable& control )
  {
    return control.polarity();
  });
  if ( !positive ) {
    throw std::runtime_error ( "Only positive polarity control lines are supported."
  }


  std::vector < metaSMT::logic::predicate > control_predicates;

  if ( controls.empty() ) {
    auto truePred = metaSMT::logic::new_variable ();
    assertion ( solver, equal ( truePred, True ) );
    control_predicates.push_back( truePred);
  }
  else if ( controls.size() == 1 ) {
    auto control_line = controls[0];
    metaSMT::logic::predicate pred = vars[control_line.line() ];
    control_predicates.push_back( pred );
  }
  else {
    for ( auto i = 0u; i < controls.size(); ++i ) {
      control_predicates.push_back ( vars[ controls[i].line() ]);
    }
  }

  return control_predicates;
}

template<typename Solver>
metaSMT::logic::predicate control_constraint (
    Solver& solver
  , std::vector < metaSMT::logic::predicate > const& controls )
{
  if ( controls.empty() || controls.size() == 1 ) {
    return controls[0];
  }
  else if ( controls.size() == 2 ) {
    auto res = metaSMT::logic::new_variable ();
    assertion ( solver, equal ( res, And ( controls[0], controls[1])));
  }
  else {
    auto current_pred = controls[0];
    for ( auto index = 0; index < controls.size(); ++index )
    {
      auto new_pred = metaSMT::logic::new_variable ();
      assertion ( solver, equal ( new_pred, And ( controls[index], current_pred)));
      current_pred = new_pred;
    }
    return current_pred;
  }
}

template<typename Solver>
metaSMT::logic::predicate or_reduce (
    Solver& solver
  , std::vector < metaSMT::logic::predicate > const& inputs
) {
  assert ( !inputs.empty() );

  if ( inputs.size() == 1 ) {
    return inputs[0];
  }
  else if ( inputs.size() == 2 ) {
    auto output = metaSMT::logic::new_variable ();

    assertion ( solver, equal ( output, Or ( inputs[0], inputs[1])) );
    return output;
  }
  else {
    auto current_pred = inputs[0];
    for ( auto index = 1; index < inputs.size(); ++index )
    {
      auto new_pred = metaSMT::logic::new_variable ();
      assertion ( solver, equal ( new_pred, Or ( inputs[index], current_pred)));
      current_pred = new_pred;
    }
    return current_pred;
  }

}

template<typename Solver>
std::vector < metaSMT::logic::predicate >
model_circuit (
    Solver& solver
  , circuit const& circ
  , std::vector < metaSMT::logic::predicate > const& vars
) {
  std::vector < metaSMT::logic::predicate > moving_vars ( vars );

  for ( auto i : boost::counting_range ( 0u, circ.num_gates() )) {
    auto const& g = circ[i];

    assert ( is_toffoli ( g ));

    auto const& C = g.controls();

    auto const& T = g.targets();
    assert ( T.size() == 1 );
    auto target_line = T[0];

    auto target_input = moving_vars[target_line];
    auto target_output = metaSMT::logic::new_variable ();
    moving_vars[target_line] = target_output;

    auto control_preds = get_control_predicates ( solver, C, moving_vars );
    auto control_contraint = control_constraint ( solver, control_preds );

    assertion ( solver
                , equal ( target_output
                , Xor ( target_input, control_contraint ) ) );

  }

  return moving_vars;
}

template<typename Solver>
class formal_equivalence_check
{
public:
  using result_type = ec_check_result;

public:
  formal_equivalence_check( circuit const& spec, circuit const& impl )
    : m_spec ( spec )
    , m_impl ( impl )
  {
    assert ( m_spec.num_gates() > 0u );
    assert ( m_impl.num_gates() > 0u );
    assert ( m_spec.lines() == m_impl.lines() );
  }

  void model_circuits()
  {
    boost::timer timer;
    auto lines = allocate_inputs ( m_spec.lines() );
    assert ( lines.size() == m_spec.lines() );

    auto spec_output = model_circuit ( m_solver, m_spec, lines );
    auto impl_output = model_circuit ( m_solver, m_impl, lines );

    std::vector < metaSMT::logic::predicate > output_constraint;

    assert ( spec_output.size() == impl_output.size() );

    for ( auto i = 0u; i < spec_output.size(); ++i ) {
      auto output = metaSMT::logic::new_variable ();

      assertion ( m_solver,
                  equal ( output, Xor ( spec_output[i], impl_output[i])));

      output_constraint.push_back( output );
    }

    auto goal = or_reduce ( m_solver, output_constraint );

    assertion ( m_solver, equal ( goal, metaSMT::logic::True ) );

    std::cout << boost::format ("[i] Circuits model: %.2f seconds")
                 % timer.elapsed()
              << std::endl;
  }

  result_type operator() () {
    model_circuits();

    if ( metaSMT::solve ( m_solver ) ) {
      std::cout << "[i] Circuits are not equivalent." << std::endl;
    } else {
      std::cout << "[i] Circuits are equivalent." << std::endl;
    }

    return ec_check_result();
  }

  private:
    Solver m_solver;
    circuit const& m_spec;
    circuit const& m_impl;
};

} // namespace revkit

#endif // HAS_METASMT

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
