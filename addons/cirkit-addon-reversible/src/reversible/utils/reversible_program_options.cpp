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

#include "reversible_program_options.hpp"

#include "costs.hpp"

namespace cirkit
{

  class reversible_program_options::priv
  {
  public:
    std::string in_realization;
    std::string in_specification;
    std::string out_realization;
    unsigned costs = 0u;

    bool has_in_realization = false;
    bool has_in_specification = false;
    bool has_out_realization = false;
    bool has_costs = false;
  };

  reversible_program_options::reversible_program_options( unsigned line_length )
    : program_options( line_length ),
      d( new priv() )
  {
  }

  reversible_program_options::reversible_program_options( const std::string& caption, unsigned line_length )
    : program_options( caption, line_length ),
      d( new priv() )
  {
  }

  reversible_program_options::~reversible_program_options()
  {
    delete d;
  }

  bool reversible_program_options::good() const
  {
    return ( program_options::good() && ( !d->has_in_realization || is_set( "filename" ) ) && ( !d->has_in_specification || is_set( "filename" ) ) );
  }

  reversible_program_options& reversible_program_options::add_read_realization_option()
  {
    assert( !( d->has_in_realization || d->has_in_specification ) );
    add_options()( "filename", boost::program_options::value( &d->in_realization ), "circuit realization in RevLib *.real format" );
    d->has_in_realization = true;

    return *this;
  }

  reversible_program_options& reversible_program_options::add_read_specification_option()
  {
    assert( !( d->has_in_realization || d->has_in_specification ) );
    add_options()( "filename", boost::program_options::value( &d->in_specification ), "circuit specification in RevLib *.spec format" );
    d->has_in_specification = true;

    return *this;
  }

  reversible_program_options& reversible_program_options::add_write_realization_option()
  {
    add_options()( "realname", boost::program_options::value( &d->out_realization ), "output circuit realization in RevLib *.real format" );
    d->has_out_realization = true;

    return *this;
  }

  reversible_program_options& reversible_program_options::add_costs_option()
  {
    add_options()( "costs", value_with_default( &d->costs ), "0: Gate Costs\n1: Line Costs\n2: Transistor Costs\n3: NCV Quantum Costs\n4: T-depth" );
    d->has_costs = true;

    return *this;
  }

  const std::string& reversible_program_options::read_realization_filename() const
  {
    return d->in_realization;
  }

  const std::string& reversible_program_options::read_specification_filename() const
  {
    return d->in_specification;
  }

  const std::string& reversible_program_options::write_realization_filename() const
  {
    return d->out_realization;
  }

  cost_function reversible_program_options::costs() const
  {
    assert( d->has_costs );

    switch ( d->costs )
    {
    case 0:
      return costs_by_circuit_func( gate_costs() );
    case 1:
      return costs_by_circuit_func( line_costs() );
    case 2:
      return costs_by_gate_func( transistor_costs() );
    case 3:
      return costs_by_gate_func( ncv_quantum_costs() );
    case 4:
      return costs_by_gate_func( t_depth_costs() );
    default:
      assert( false );
      return cost_function();
    }
  }

  bool reversible_program_options::is_write_realization_filename_set() const
  {
    if ( !parsed() || !d->has_out_realization )
    {
      return false;
    }

    return is_set( "realname" );
  }
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
