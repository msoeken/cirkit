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

#include <boost/range/adaptors.hpp>

#include <fmt/format.h>

#include "gate.hpp"

#include "functions/copy_circuit.hpp"

namespace cirkit
{
  unsigned circuit::num_gates() const
  {
    return gates.size();
  }

  void circuit::set_lines( unsigned lines )
  {
    _inputs.resize( lines );
    _outputs.resize( lines );
    for ( unsigned i = lines; i < lines; ++i )
    {
      _inputs[i] = fmt::format( "i{}", i );
      _outputs[i] = fmt::format( "o{}", i );
    }
    _constants.resize( lines, constant() );
    _garbage.resize( lines, false );
    _lines = lines; 
  }

  unsigned circuit::lines() const
  {
    return _lines;
  }

  circuit::const_iterator circuit::begin() const
  {
    return boost::make_indirect_iterator( gates.begin() );
  }

  circuit::const_iterator circuit::end() const
  {
    return boost::make_indirect_iterator( gates.end() );
  }

  circuit::iterator circuit::begin()
  {
    return boost::make_indirect_iterator( gates.begin() );
  }

  circuit::iterator circuit::end()
  {
    return boost::make_indirect_iterator( gates.end() );
  }

  circuit::const_reverse_iterator circuit::rbegin() const
  {
    return boost::make_indirect_iterator( gates.rbegin() );
  }

  circuit::const_reverse_iterator circuit::rend() const
  {
    return boost::make_indirect_iterator( gates.rend() );
  }

  circuit::reverse_iterator circuit::rbegin()
  {
    return boost::make_indirect_iterator( gates.rbegin() );
  }

  circuit::reverse_iterator circuit::rend()
  {
    return boost::make_indirect_iterator( gates.rend() );
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
    gates.push_back( std::make_shared<gate>() );
    return *gates.back();
  }

  gate& circuit::prepend_gate()
  {
    gates.insert( gates.begin(), std::make_shared<gate>() );
    return *gates.front();
  }

  gate& circuit::insert_gate( unsigned pos )
  {
    gates.insert( gates.begin() + pos, std::make_shared<gate>() );
    return *gates.at( pos );
  }

  void circuit::remove_gate_at( unsigned pos )
  {
    if ( pos < gates.size() )
    {
      gates.erase( gates.begin() + pos );
    }
  }

  void circuit::set_inputs( const std::vector<std::string>& inputs )
  {
    _inputs.clear();
    std::copy( inputs.begin(), inputs.end(), std::back_inserter( _inputs ) );
    _inputs.resize( _lines, "i" );
  }

  const std::vector<std::string>& circuit::inputs() const
  {
    return _inputs;
  }

  void circuit::set_outputs( const std::vector<std::string>& outputs )
  {
    _outputs.clear();
    std::copy( outputs.begin(), outputs.end(), std::back_inserter( _outputs ) );
    _outputs.resize( _lines, "o" );
  }

  const std::vector<std::string>& circuit::outputs() const
  {
    return _outputs;
  }

  void circuit::set_constants( const std::vector<constant>& constants )
  {
    _constants.clear();
    std::copy( constants.begin(), constants.end(), std::back_inserter( _constants ) );
    _constants.resize( _lines, constant() );
  }

  const std::vector<constant>& circuit::constants() const
  {
    return _constants;
  }

  void circuit::set_garbage( const std::vector<bool>& garbage )
  {
    _garbage.clear();
    std::copy( garbage.begin(), garbage.end(), std::back_inserter( _garbage ) );
    _garbage.resize( _lines, false );
  }

  const std::vector<bool>& circuit::garbage() const
  {
    return _garbage;
  }

  void circuit::set_circuit_name( const std::string& name )
  {
    _name = name;
  }

  const std::string& circuit::circuit_name() const
  {
    return _name;
  }

  const bus_collection& circuit::inputbuses() const
  {
    return _inputbuses;
  }

  bus_collection& circuit::inputbuses()
  {
    return _inputbuses;
  }

  const bus_collection& circuit::outputbuses() const
  {
    return _outputbuses;
  }

  bus_collection& circuit::outputbuses()
  {
    return _outputbuses;
  }

  const bus_collection& circuit::statesignals() const
  {
    return _statesignals;
  }

  bus_collection& circuit::statesignals()
  {
    return _statesignals;
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
    std::map<const gate*, std::map<std::string, std::string> >::const_iterator it = _annotations.find( &g );
    if ( it != _annotations.end() )
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

  boost::optional<const std::map<std::string, std::string>& > circuit::annotations( const gate& g ) const
  {
    std::map<const gate*, std::map<std::string, std::string> >::const_iterator it = _annotations.find( &g );
    if ( it != _annotations.end() )
    {
      return boost::optional<const std::map<std::string, std::string>& >( it->second );
    }
    else
    {
      return boost::optional<const std::map<std::string, std::string>& >();
    }
  }

  void circuit::annotate( const gate& g, const std::string& key, const std::string& value )
  {
    _annotations[&g][key] = value;
  }
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
