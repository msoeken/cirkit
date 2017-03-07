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

/**
 * @file rcbdd.hpp
 *
 * @brief Data structure for BDD of a characteristic function to a reversible function
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef RCBDD_HPP
#define RCBDD_HPP

#include <boost/assign/std/vector.hpp>
#include <boost/optional.hpp>

#include <cuddObj.hh>

#include <reversible/circuit.hpp>

namespace cirkit
{

  using namespace boost::assign;

  class rcbdd
  {
  public:
    void initialize_manager();
    void create_variables( unsigned n, bool create_zs = true );
    BDD x( unsigned i ) const;
    BDD y( unsigned i ) const;
    BDD z( unsigned i ) const;

    const std::vector<BDD> xs() const;
    const std::vector<BDD> ys() const;
    const std::vector<BDD> zs() const;

    unsigned num_vars() const;
    const Cudd& manager() const;
    BDD chi() const;
    void set_chi( BDD f );
    void set_constant_value( bool v );
    bool constant_value() const;
    void set_num_inputs( unsigned n );
    void set_num_outputs( unsigned n );
    unsigned num_inputs() const;
    unsigned num_outputs() const;
    void set_input_labels( const std::vector<std::string>& labels );
    void set_output_labels( const std::vector<std::string>& labels );
    const std::vector<std::string> input_labels() const;
    const std::vector<std::string> output_labels() const;

    BDD compose(const BDD& left, const BDD& right) const;
    BDD cofactor( BDD f, unsigned var, bool input_polarity, bool output_polarity ) const;
    BDD move_xs_to_tmp( const BDD& f ) const;
    BDD move_ys_to_tmp( const BDD& f ) const;
    BDD move_tmp_to_ys( const BDD& f ) const;
    BDD move_ys_to_xs( const BDD& f) const;
    BDD remove_xs( const BDD& f ) const;
    BDD remove_ys( const BDD& f ) const;
    BDD remove_tmp( const BDD& f ) const;
    BDD invert( const BDD&f ) const;
    bool is_self_inverse( const BDD& f ) const;

    BDD create_from_gate( unsigned target, const BDD& controlf ) const;
    BDD create_from_gate( const gate& g ) const;
    BDD create_from_circuit( const circuit& circ ) const;

    void print_truth_table() const;
    void write_pla( const std::string& filename, bool full = false ) const;

  private:
    boost::optional<Cudd> _manager;
    BDD _chi;

    bool _constant_value = false;
    unsigned _num_inputs = 0u;
    unsigned _num_outputs = 0u;
    std::vector<std::string> _input_labels;
    std::vector<std::string> _output_labels;
    unsigned _n = 0u;
    std::vector<BDD> _xs;
    std::vector<BDD> _ys;
    std::vector<BDD> _zs;
  };

  void copy_meta_data( circuit& circ, const rcbdd& cf );
}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
