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

#include "circuit.hpp"

#include <iostream>
#include <string>

#include <boost/format.hpp>
#include <boost/range/adaptors.hpp>

#include "gate.hpp"

#include "functions/copy_circuit.hpp"

namespace cirkit
{
  using boost::adaptors::indirected;
  using boost::adaptors::transformed;

  struct num_gates_visitor : public boost::static_visitor<unsigned>
  {
    unsigned operator()( const standard_circuit& circ ) const
    {
      return circ.gates.size();
    }

    unsigned operator()( const subcircuit& circ ) const
    {
      return circ.to - circ.from;
    }
  };

  struct lines_setter : public boost::static_visitor<>
  {
    explicit lines_setter( unsigned _lines ) : lines( _lines ) {}

    void operator()( standard_circuit& circ ) const
    {
      circ.inputs.resize( lines );
      circ.outputs.resize( lines );
      for ( unsigned i = circ.lines; i < lines; ++i )
      {
          circ.inputs[i] = boost::str( boost::format( "i%d" ) % i );
          circ.outputs[i] = boost::str( boost::format( "o%d" ) % i );
      }
      circ.constants.resize( lines, constant() );
      circ.garbage.resize( lines, false );
      circ.lines = lines;
    }

    void operator()( subcircuit& circ ) const
    {
      // NOTE expand the sub-circuit and therewith automatically the base circuit (in future version)
      assert( false );
    }

  private:
    unsigned lines;
  };

  struct lines_visitor : public boost::static_visitor<unsigned>
  {
    unsigned operator()( const standard_circuit& circ ) const
    {
      return circ.lines;
    }

    unsigned operator()( const subcircuit& circ ) const
    {
      return circ.base->lines;
    }
  };

  struct const_begin_visitor : public boost::static_visitor<circuit::const_iterator>
  {
    circuit::const_iterator operator()( const standard_circuit& circ ) const
    {
      return boost::make_indirect_iterator( circ.gates.begin() );
    }

    circuit::const_iterator operator()( const subcircuit& circ ) const
    {
      return boost::make_indirect_iterator( circ.base->gates.begin() + circ.from );
    }
  };

  struct const_end_visitor : public boost::static_visitor<circuit::const_iterator>
  {
    circuit::const_iterator operator()( const standard_circuit& circ ) const
    {
      return boost::make_indirect_iterator( circ.gates.end() );
    }

    circuit::const_iterator operator()( const subcircuit& circ ) const
    {
      return boost::make_indirect_iterator( circ.base->gates.begin() + circ.to );
    }
  };

  struct begin_visitor : public boost::static_visitor<circuit::iterator>
  {
    circuit::iterator operator()( standard_circuit& circ ) const
    {
      return boost::make_indirect_iterator( circ.gates.begin() );
    }

    circuit::iterator operator()( subcircuit& circ ) const
    {
      return boost::make_indirect_iterator( circ.base->gates.begin() + circ.from );
    }
  };

  struct end_visitor : public boost::static_visitor<circuit::iterator>
  {
    circuit::iterator operator()( standard_circuit& circ ) const
    {
      return boost::make_indirect_iterator( circ.gates.end() );
    }

    circuit::iterator operator()( subcircuit& circ ) const
    {
      return boost::make_indirect_iterator( circ.base->gates.begin() + circ.to );
    }
  };

  struct const_rbegin_visitor : public boost::static_visitor<circuit::const_reverse_iterator>
  {
    circuit::const_reverse_iterator operator()( const standard_circuit& circ ) const
    {
      return boost::make_indirect_iterator( circ.gates.rbegin() );
    }

    circuit::const_reverse_iterator operator()( const subcircuit& circ ) const
    {
      return boost::make_indirect_iterator( circ.base->gates.rbegin() + ( circ.base->gates.size() - circ.to ) );
    }
  };

  struct const_rend_visitor : public boost::static_visitor<circuit::const_reverse_iterator>
  {
    circuit::const_reverse_iterator operator()( const standard_circuit& circ ) const
    {
      return boost::make_indirect_iterator( circ.gates.rend() );
    }

    circuit::const_reverse_iterator operator()( const subcircuit& circ ) const
    {
      return boost::make_indirect_iterator( circ.base->gates.rbegin() + ( circ.base->gates.size() - circ.from ) );
    }
  };

  struct rbegin_visitor : public boost::static_visitor<circuit::reverse_iterator>
  {
    circuit::reverse_iterator operator()( standard_circuit& circ ) const
    {
      return boost::make_indirect_iterator( circ.gates.rbegin() );
    }

    circuit::reverse_iterator operator()( subcircuit& circ ) const
    {
      return boost::make_indirect_iterator( circ.base->gates.rbegin() + ( circ.base->gates.size() - circ.to ) );
    }
  };

  struct rend_visitor : public boost::static_visitor<circuit::reverse_iterator>
  {
    circuit::reverse_iterator operator()( standard_circuit& circ ) const
    {
      return boost::make_indirect_iterator( circ.gates.rend() );
    }

    circuit::reverse_iterator operator()( subcircuit& circ ) const
    {
      return boost::make_indirect_iterator( circ.base->gates.rbegin() + ( circ.base->gates.size() - circ.from ) );
    }
  };

  struct append_gate_visitor : public boost::static_visitor<gate&>
  {
    gate& operator()( standard_circuit& circ ) const
    {
      circ.gates.push_back( std::make_shared<gate>() );
      return *circ.gates.back();
    }

    gate& operator()( subcircuit& circ ) const
    {
      circ.base->gates.insert( circ.base->gates.begin() + circ.to, std::make_shared<gate>() );
      ++circ.to;

      gate& g = **( circ.base->gates.begin() + circ.to - 1 );
      return g;
    }
  };

  struct prepend_gate_visitor : public boost::static_visitor<gate&>
  {
    gate& operator()( standard_circuit& circ ) const
    {
      circ.gates.insert( circ.gates.begin(), std::make_shared<gate>() );
      return *circ.gates.front();
    }

    gate& operator()( subcircuit& circ ) const
    {
      circ.base->gates.insert( circ.base->gates.begin() + circ.from, std::make_shared<gate>() );
      ++circ.to;

      gate& g = **( circ.base->gates.begin() + circ.from );
      return g;
    }
  };

  struct insert_gate_visitor : public boost::static_visitor<gate&>
  {
    explicit insert_gate_visitor( unsigned _pos ) : pos( _pos ) {}

    gate& operator()( standard_circuit& circ ) const
    {
      circ.gates.insert( circ.gates.begin() + pos, std::make_shared<gate>() );
      return *circ.gates.at( pos );
    }

    gate& operator()( subcircuit& circ ) const
    {
      circ.base->gates.insert( circ.base->gates.begin() + circ.from + pos, std::make_shared<gate>() );
      ++circ.to;

      gate& g = **( circ.base->gates.begin() + circ.from + pos );
      return g;
    }

  private:
    unsigned pos;
  };

  struct remove_gate_at_visitor : public boost::static_visitor<>
  {
    explicit remove_gate_at_visitor( unsigned _pos ) : pos( _pos ) {}

    void operator()( standard_circuit& circ ) const
    {
      if ( pos < circ.gates.size() )
      {
        circ.gates.erase( circ.gates.begin() + pos );
      }
    }

    void operator()( subcircuit& circ ) const
    {
      if ( pos < circ.to )
      {
        circ.base->gates.erase( circ.base->gates.begin() + circ.from + pos );
        --circ.to;
      }
    }

  private:
    unsigned pos;
  };

  struct inputs_setter : public boost::static_visitor<>
  {
    explicit inputs_setter( const std::vector<std::string>& _inputs ) : inputs( _inputs ) {}

    void operator()( standard_circuit& circ ) const
    {
      circ.inputs.clear();
      std::copy( inputs.begin(), inputs.end(), std::back_inserter( circ.inputs ) );
      circ.inputs.resize( circ.lines, "i" );
    }

    void operator()( subcircuit& circ ) const
    {
      circ.base->inputs.clear();
      std::copy( inputs.begin(), inputs.end(), std::back_inserter( circ.base->inputs ) );
      circ.base->inputs.resize( circ.base->lines, "i" );
    }

  private:
    const std::vector<std::string>& inputs;
  };

  struct inputs_visitor : public boost::static_visitor<const std::vector<std::string>& >
  {
    const std::vector<std::string>& operator()( const standard_circuit& circ ) const
    {
      return circ.inputs;
    }

    const std::vector<std::string>& operator()( const subcircuit& circ ) const
    {
      return circ.base->inputs;
    }
  };

  struct outputs_setter : public boost::static_visitor<>
  {
    explicit outputs_setter( const std::vector<std::string>& _outputs ) : outputs( _outputs ) {}

    void operator()( standard_circuit& circ ) const
    {
      circ.outputs.clear();
      std::copy( outputs.begin(), outputs.end(), std::back_inserter( circ.outputs ) );
      circ.outputs.resize( circ.lines, "o" );
    }

    void operator()( subcircuit& circ ) const
    {
      circ.base->outputs.clear();
      std::copy( outputs.begin(), outputs.end(), std::back_inserter( circ.base->outputs ) );
      circ.base->outputs.resize( circ.base->lines, "o" );
    }

  private:
    const std::vector<std::string>& outputs;
  };

  struct outputs_visitor : public boost::static_visitor<const std::vector<std::string>& >
  {
    const std::vector<std::string>& operator()( const standard_circuit& circ ) const
    {
      return circ.outputs;
    }

    const std::vector<std::string>& operator()( const subcircuit& circ ) const
    {
      return circ.base->outputs;
    }
  };

  struct constants_setter : public boost::static_visitor<>
  {
    explicit constants_setter( const std::vector<constant>& _constants ) : constants( _constants ) {}

    void operator()( standard_circuit& circ ) const
    {
      circ.constants.clear();
      std::copy( constants.begin(), constants.end(), std::back_inserter( circ.constants ) );
      circ.constants.resize( circ.lines, constant() );
    }

    void operator()( subcircuit& circ ) const
    {
      circ.base->constants.clear();
      std::copy( constants.begin(), constants.end(), std::back_inserter( circ.base->constants ) );
      circ.base->constants.resize( circ.base->lines, constant() );
    }

  private:
    const std::vector<constant>& constants;
  };

  struct constants_visitor : public boost::static_visitor<const std::vector<constant>& >
  {
    const std::vector<constant>& operator()( const standard_circuit& circ ) const
    {
      return circ.constants;
    }

    const std::vector<constant>& operator()( const subcircuit& circ ) const
    {
      return circ.base->constants;
    }
  };

  struct garbage_setter : public boost::static_visitor<>
  {
    explicit garbage_setter( const std::vector<bool>& _garbage ) : garbage( _garbage ) {}

    void operator()( standard_circuit& circ ) const
    {
      circ.garbage.clear();
      std::copy( garbage.begin(), garbage.end(), std::back_inserter( circ.garbage ) );
      circ.garbage.resize( circ.lines, false );
    }

    void operator()( subcircuit& circ ) const
    {
      circ.base->garbage.clear();
      std::copy( garbage.begin(), garbage.end(), std::back_inserter( circ.base->garbage ) );
      circ.base->garbage.resize( circ.base->lines, false );
    }

  private:
    const std::vector<bool>& garbage;
  };

  struct garbage_visitor : public boost::static_visitor<const std::vector<bool>& >
  {
    const std::vector<bool>& operator()( const standard_circuit& circ ) const
    {
      return circ.garbage;
    }

    const std::vector<bool>& operator()( const subcircuit& circ ) const
    {
      return circ.base->garbage;
    }
  };

  struct circuit_name_setter : public boost::static_visitor<>
  {
    explicit circuit_name_setter( const std::string& _name ) : name( _name ) {}

    void operator()( standard_circuit& circ ) const
    {
      circ.name = name;
    }

    void operator()( subcircuit& circ ) const
    {
      circ.base->name = name;
    }

  private:
    const std::string& name;
  };

  struct circuit_name_visitor : public boost::static_visitor<const std::string&>
  {
    const std::string& operator()( const standard_circuit& circ ) const
    {
      return circ.name;
    }

    const std::string& operator()( const subcircuit& circ ) const
    {
      return circ.base->name;
    }
  };

  struct const_inputbuses_visitor : public boost::static_visitor<const bus_collection&>
  {
    const bus_collection& operator()( const standard_circuit& circ ) const
    {
      return circ.inputbuses;
    }

    const bus_collection& operator()( const subcircuit& circ ) const
    {
      return circ.base->inputbuses;
    }
  };

  struct inputbuses_visitor : public boost::static_visitor<bus_collection&>
  {
    bus_collection& operator()( standard_circuit& circ ) const
    {
      return circ.inputbuses;
    }

    bus_collection& operator()( subcircuit& circ ) const
    {
      return circ.base->inputbuses;
    }
  };

  struct const_outputbuses_visitor : public boost::static_visitor<const bus_collection&>
  {
    const bus_collection& operator()( const standard_circuit& circ ) const
    {
      return circ.outputbuses;
    }

    const bus_collection& operator()( const subcircuit& circ ) const
    {
      return circ.base->outputbuses;
    }
  };

  struct outputbuses_visitor : public boost::static_visitor<bus_collection&>
  {
    bus_collection& operator()( standard_circuit& circ ) const
    {
      return circ.outputbuses;
    }

    bus_collection& operator()( subcircuit& circ ) const
    {
      return circ.base->outputbuses;
    }
  };

  struct const_statesignals_visitor : public boost::static_visitor<const bus_collection&>
  {
    const bus_collection& operator()( const standard_circuit& circ ) const
    {
      return circ.statesignals;
    }

    const bus_collection& operator()( const subcircuit& circ ) const
    {
      return circ.base->statesignals;
    }
  };

  struct statesignals_visitor : public boost::static_visitor<bus_collection&>
  {
    bus_collection& operator()( standard_circuit& circ ) const
    {
      return circ.statesignals;
    }

    bus_collection& operator()( subcircuit& circ ) const
    {
      return circ.base->statesignals;
    }
  };

  struct is_subcircuit_visitor : public boost::static_visitor<bool>
  {
    bool operator()( const standard_circuit& circ ) const
    {
      return false;
    }

    bool operator()( const subcircuit& circ ) const
    {
      return true;
    }
  };

  struct offset_visitor : public boost::static_visitor<unsigned>
  {
    unsigned operator()( const standard_circuit& circ ) const
    {
      return 0u;
    }

    unsigned operator()( const subcircuit& circ ) const
    {
      return circ.from;
    }
  };

  struct annotation_visitor : public boost::static_visitor<const std::string&>
  {
    annotation_visitor( const gate& g, const std::string& key, const std::string& default_value )
      : g( g ), key( key ), default_value( default_value )
    {
    }

    const std::string& operator()( const standard_circuit& circ ) const
    {
      std::map<const gate*, std::map<std::string, std::string> >::const_iterator it = circ.annotations.find( &g );
      if ( it != circ.annotations.end() )
      {
        std::map<std::string, std::string>::const_iterator it2 = it->second.find( key );
        if ( it2 != it->second.end() )
        {
          return it2->second;
        }
        else
        {
          return default_value;
        }
      }
      else
      {
        return default_value;
      }
    }

    const std::string& operator()( const subcircuit& circ ) const
    {
      return operator()( *circ.base );
    }

  private:
    const gate& g;
    const std::string& key;
    const std::string& default_value;
  };

  struct annotations_visitor : public boost::static_visitor<boost::optional<const std::map<std::string, std::string>& > >
  {
    explicit annotations_visitor( const gate& g ) : g( g ) {}

    boost::optional<const std::map<std::string, std::string>& > operator()( const standard_circuit& circ ) const
    {
      std::map<const gate*, std::map<std::string, std::string> >::const_iterator it = circ.annotations.find( &g );
      if ( it != circ.annotations.end() )
      {
        return boost::optional<const std::map<std::string, std::string>& >( it->second );
      }
      else
      {
        return boost::optional<const std::map<std::string, std::string>& >();
      }
    }

    boost::optional<const std::map<std::string, std::string>& > operator()( const subcircuit& circ ) const
    {
      return operator()( circ );
    }

  private:
    const gate& g;
  };

  struct annotate_visitor : public boost::static_visitor<>
  {
    annotate_visitor( const gate& g, const std::string& key, const std::string& value )
      : g( g ), key( key ), value( value )
    {
    }

    void operator()( standard_circuit& circ ) const
    {
      circ.annotations[&g][key] = value;
    }

    void operator()( subcircuit& circ ) const
    {
      operator()( *circ.base );
    }

  private:
    const gate& g;
    const std::string& key;
    const std::string& value;
  };

  unsigned circuit::num_gates() const
  {
    return boost::apply_visitor( num_gates_visitor(), circ );
  }

  void circuit::set_lines( unsigned lines )
  {
    boost::apply_visitor( lines_setter( lines ), circ );
  }

  unsigned circuit::lines() const
  {
    return boost::apply_visitor( lines_visitor(), circ );
  }

  circuit::const_iterator circuit::begin() const
  {
    return boost::apply_visitor( const_begin_visitor(), circ );
  }

  circuit::const_iterator circuit::end() const
  {
    return boost::apply_visitor( const_end_visitor(), circ );
  }

  circuit::iterator circuit::begin()
  {
    return boost::apply_visitor( begin_visitor(), circ );
  }

  circuit::iterator circuit::end()
  {
    return boost::apply_visitor( end_visitor(), circ );
  }

  circuit::const_reverse_iterator circuit::rbegin() const
  {
    return boost::apply_visitor( const_rbegin_visitor(), circ );
  }

  circuit::const_reverse_iterator circuit::rend() const
  {
    return boost::apply_visitor( const_rend_visitor(), circ );
  }

  circuit::reverse_iterator circuit::rbegin()
  {
    return boost::apply_visitor( rbegin_visitor(), circ );
  }

  circuit::reverse_iterator circuit::rend()
  {
    return boost::apply_visitor( rend_visitor(), circ );
  }

  const gate& circuit::operator[]( unsigned index ) const
  {
    return *( begin() + index );
  }

  gate& circuit::operator[]( unsigned index )
  {
    return *( begin() + index );
  }

  gate& circuit::append_gate()
  {
    return boost::apply_visitor( append_gate_visitor(), circ );
  }

  gate& circuit::prepend_gate()
  {
    return boost::apply_visitor( prepend_gate_visitor(), circ );
  }

  gate& circuit::insert_gate( unsigned pos )
  {
    return boost::apply_visitor( insert_gate_visitor( pos ), circ );
  }

  void circuit::remove_gate_at( unsigned pos )
  {
    boost::apply_visitor( remove_gate_at_visitor( pos ), circ );
  }

  void circuit::set_inputs( const std::vector<std::string>& inputs )
  {
    boost::apply_visitor( inputs_setter( inputs ), circ );
  }

  const std::vector<std::string>& circuit::inputs() const
  {
    return boost::apply_visitor( inputs_visitor(), circ );
  }

  void circuit::set_outputs( const std::vector<std::string>& outputs )
  {
    boost::apply_visitor( outputs_setter( outputs ), circ );
  }

  const std::vector<std::string>& circuit::outputs() const
  {
    return boost::apply_visitor( outputs_visitor(), circ );
  }

  void circuit::set_constants( const std::vector<constant>& constants )
  {
    boost::apply_visitor( constants_setter( constants ), circ );
  }

  const std::vector<constant>& circuit::constants() const
  {
    return boost::apply_visitor( constants_visitor(), circ );
  }

  void circuit::set_garbage( const std::vector<bool>& garbage )
  {
    boost::apply_visitor( garbage_setter( garbage ), circ );
  }

  const std::vector<bool>& circuit::garbage() const
  {
    return boost::apply_visitor( garbage_visitor(), circ );
  }

  void circuit::set_circuit_name( const std::string& name )
  {
    boost::apply_visitor( circuit_name_setter( name ), circ );
  }

  const std::string& circuit::circuit_name() const
  {
    return boost::apply_visitor( circuit_name_visitor(), circ );
  }

  const bus_collection& circuit::inputbuses() const
  {
    return boost::apply_visitor( const_inputbuses_visitor(), circ );
  }

  bus_collection& circuit::inputbuses()
  {
    return boost::apply_visitor( inputbuses_visitor(), circ );
  }

  const bus_collection& circuit::outputbuses() const
  {
    return boost::apply_visitor( const_outputbuses_visitor(), circ );
  }

  bus_collection& circuit::outputbuses()
  {
    return boost::apply_visitor( outputbuses_visitor(), circ );
  }

  const bus_collection& circuit::statesignals() const
  {
    return boost::apply_visitor( const_statesignals_visitor(), circ );
  }

  bus_collection& circuit::statesignals()
  {
    return boost::apply_visitor( statesignals_visitor(), circ );
  }

  bool circuit::is_subcircuit() const
  {
    return boost::apply_visitor( is_subcircuit_visitor(), circ );
  }

  unsigned circuit::offset() const
  {
    return boost::apply_visitor( offset_visitor(), circ );
  }

  void circuit::add_module( const std::string& name, const std::shared_ptr<circuit>& module )
  {
    _modules.insert( std::make_pair( name, module ) );
  }

  void circuit::add_module( const std::string& name, const circuit& module )
  {
    circuit* copy = new circuit();
    copy_circuit( module, *copy );
    add_module( name, std::shared_ptr<circuit>( copy ) );
  }

  const std::map<std::string, std::shared_ptr<circuit> >& circuit::modules() const
  {
    return _modules;
  }

  const std::string& circuit::annotation( const gate& g, const std::string& key, const std::string& default_value ) const
  {
    return boost::apply_visitor( annotation_visitor( g, key, default_value ), circ );
  }

  boost::optional<const std::map<std::string, std::string>& > circuit::annotations( const gate& g ) const
  {
    return boost::apply_visitor( annotations_visitor( g ), circ );
  }


  void circuit::annotate( const gate& g, const std::string& key, const std::string& value )
  {
    boost::apply_visitor( annotate_visitor( g, key, value ), circ );
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
