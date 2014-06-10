/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include "read_realization.hpp"

#include <fstream>
#include <iostream>
#include <stack>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/regex.hpp>
#include <boost/variant.hpp>

#include "revlib_parser.hpp"
#include "print_circuit.hpp"
#include "../target_tags.hpp"
#include "../functions/add_gates.hpp"

using namespace boost::assign;

namespace revkit
{

  ////////////////////////////// class circuit_processor
  class circuit_processor::priv
  {
  public:
    priv( circuit& c )
    {
      circs.push( &c );
    }

    std::stack<circuit*> circs;
  };

  circuit_processor::circuit_processor( circuit& circ )
    : revlib_processor(), d( new priv( circ ) )
  {
  }

  circuit_processor::~circuit_processor()
  {
    delete d;
  }

  void circuit_processor::on_comment( const std::string& comment ) const
  {
  }

  void circuit_processor::on_numvars( unsigned numvars ) const
  {
    d->circs.top()->set_lines( numvars );
  }

  void circuit_processor::on_inputs( std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last ) const
  {
    std::vector<std::string> inputs( first, last );
    d->circs.top()->set_inputs( inputs );
  }

  void circuit_processor::on_outputs( std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last ) const
  {
    std::vector<std::string> outputs( first, last );
    d->circs.top()->set_outputs( outputs );
  }

  void circuit_processor::on_constants( std::vector<constant>::const_iterator first, std::vector<constant>::const_iterator last ) const
  {
    std::vector<constant> constants( first, last );
    d->circs.top()->set_constants( constants );
  }

  void circuit_processor::on_garbage( std::vector<bool>::const_iterator first, std::vector<bool>::const_iterator last ) const
  {
    std::vector<bool> garbage( first, last );
    d->circs.top()->set_garbage( garbage );
  }

  void circuit_processor::on_inputbus( const std::string& name, const std::vector<unsigned>& line_indices ) const
  {
    d->circs.top()->inputbuses().add( name, line_indices );
  }

  void circuit_processor::on_outputbus( const std::string& name, const std::vector<unsigned>& line_indices ) const
  {
    d->circs.top()->outputbuses().add( name, line_indices );
  }

  void circuit_processor::on_state( const std::string& name, const std::vector<unsigned>& line_indices, unsigned initial_value ) const
  {
    d->circs.top()->statesignals().add( name, line_indices );
  }

  void circuit_processor::on_module( const std::string& name, const boost::optional<std::string>& filename ) const
  {
    if ( filename )
    {
      circuit* module = new circuit();
      read_realization( *module, *filename );

      d->circs.top()->add_module( name, std::shared_ptr<circuit>( module ) );
    }
    else
    {
      circuit* module = new circuit();

      d->circs.top()->add_module( name, std::shared_ptr<circuit>( module ) );
      d->circs.push( module );
    }
  }

  void circuit_processor::on_gate( const boost::any& target_type, const std::vector<variable>& line_indices ) const
  {
    assert( line_indices.back().polarity() ); /* target line must be positive */
    gate::control_container controls;
    gate* added_gate = 0;

    if ( is_type<toffoli_tag>( target_type ) )
    {
      assert( line_indices.size() > 0 );
      std::copy( line_indices.begin(), line_indices.end() - 1, std::back_inserter( controls ) );
      added_gate = &append_toffoli( *d->circs.top(), controls, line_indices.back().line() );
    }
    else if ( is_type<fredkin_tag>( target_type ) )
    {
      assert( line_indices.size() > 1 );
      std::copy( line_indices.begin(), line_indices.end() - 2, std::back_inserter( controls ) );
      added_gate = &append_fredkin( *d->circs.top(), controls, ( line_indices.end() - 2 )->line(), ( line_indices.end() - 1 )->line() );
    }
    else if ( is_type<peres_tag>( target_type ) )
    {
      assert( line_indices.size() == 3 );
      added_gate = &append_peres( *d->circs.top(), line_indices.at( 0 ), line_indices.at( 1 ).line(), line_indices.at( 2 ).line() );
    }
    else if ( is_type<module_tag>( target_type ) )
    {
      module_tag module = boost::any_cast<module_tag>( target_type );
      std::shared_ptr<circuit> module_circuit = d->circs.top()->modules().find( module.name )->second;
      assert( line_indices.size() >= module_circuit->lines() );
      module.reference = module_circuit;

      /* new gate */
      gate& module_gate = d->circs.top()->append_gate();

      /* control lines */
      unsigned num_controls = line_indices.size() - module_circuit->lines();
      std::for_each( line_indices.begin(), line_indices.begin() + num_controls, [&module_gate]( variable l ) { module_gate.add_control( l ); } );

      /* sort order */
      std::for_each( line_indices.begin() + num_controls, line_indices.end(), [&module_gate]( variable l ) { assert( l.polarity() ); module_gate.add_target( l.line() ); } );
      module_gate.set_type( module );

      added_gate = &module_gate;
    }

    /* Annotations */
    if ( added_gate )
    {
      const properties& p = *( current_annotations().get() );
      properties::storage_type::const_iterator it;
      for ( it = p.begin(); it != p.end(); ++it )
      {
        d->circs.top()->annotate( *added_gate, it->first, boost::any_cast<std::string>( it->second ) );
      }
    }
  }

  void circuit_processor::on_end() const
  {
    d->circs.pop();
  }

  bool read_realization( circuit& circ, std::istream& in, const read_realization_settings& settings, std::string* error )
  {
    circuit_processor processor( circ );

    return revlib_parser( in, processor, ".", settings.read_gates, error );
  }

  bool read_realization( circuit& circ, const std::string& filename, const read_realization_settings& settings, std::string* error )
  {
    std::ifstream is;
    is.open( filename.c_str(), std::ifstream::in );

    if ( !is.good() )
    {
      if ( error )
      {
        *error = "Cannot open " + filename;
      }
      return false;
    }

    boost::filesystem::path pfilename( filename );
    circuit_processor processor( circ );
    return revlib_parser( is, processor, pfilename.parent_path().string(), settings.read_gates, error );
  }

}

// Local Variables:
// c-basic-offset: 2
// End:
