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

    void print_truth_table();
    void write_pla( const std::string& filename );

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
