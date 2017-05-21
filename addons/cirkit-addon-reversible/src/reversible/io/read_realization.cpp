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

#include "read_realization.hpp"

#include <fstream>
#include <iostream>
#include <stack>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/variant.hpp>

#include <classical/utils/truth_table_utils.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/io/revlib_parser.hpp>
#include <reversible/io/print_circuit.hpp>

using namespace boost::assign;

namespace cirkit
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
    else
    {
      added_gate = &d->circs.top()->append_gate();
      std::for_each( line_indices.begin(), line_indices.end() - 1, [added_gate]( variable l ) { added_gate->add_control( l ); } );
      added_gate->add_target( line_indices.back().line() );
      added_gate->set_type( target_type );
    }

    /* Annotations */
    if ( added_gate )
    {
      const properties& p = *( current_annotations().get() );
      properties::storage_type::const_iterator it;
      for ( it = p.begin(); it != p.end(); ++it )
      {
        d->circs.top()->annotate( *added_gate, it->first, boost::any_cast<std::string>( it->second ) );

        /* special (active) annotations */
        if ( is_type<stg_tag>( target_type ) && it->first == "affine" )
        {
          auto stg = boost::any_cast<stg_tag>( added_gate->type() );
          stg.affine_class = tt_from_hex( boost::any_cast<std::string>( it->second ) );
          added_gate->set_type( stg );
        }
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

    revlib_parser_settings rp_settings;
    rp_settings.read_gates = settings.read_gates;
    rp_settings.string_to_target_tag = settings.string_to_target_tag;

    return revlib_parser( in, processor, rp_settings, error );
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
    revlib_parser_settings rp_settings;
    rp_settings.base_directory = pfilename.parent_path().string();
    rp_settings.read_gates = settings.read_gates;
    rp_settings.string_to_target_tag = settings.string_to_target_tag;
    return revlib_parser( is, processor, rp_settings, error );
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
