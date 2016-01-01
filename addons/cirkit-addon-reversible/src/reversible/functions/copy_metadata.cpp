/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

#include "copy_metadata.hpp"

namespace cirkit
{

  copy_metadata_settings::copy_metadata_settings()
    : copy_inputs( true ),
      copy_outputs( true ),
      copy_constants( true ),
      copy_garbage( true ),
      copy_name( true ),
      copy_inputbuses( true ),
      copy_outputbuses( true ),
      copy_statesignals( true ),
      copy_modules( true )
  {
  }

  void copy_metadata( const circuit& base, circuit& circ, const copy_metadata_settings& settings )
  {
    circ.set_lines( base.lines() );

    if ( settings.copy_inputs )    circ.set_inputs( base.inputs () );
    if ( settings.copy_outputs )   circ.set_outputs( base.outputs() );
    if ( settings.copy_constants ) circ.set_constants( base.constants() );
    if ( settings.copy_garbage )   circ.set_garbage( base.garbage() );
    if ( settings.copy_name )      circ.set_circuit_name( base.circuit_name() );

    if ( settings.copy_inputbuses )
    {
      for ( const auto& p : base.inputbuses().buses() )
      {
        circ.inputbuses().add( p.first, p.second );
      }
    }
    if ( settings.copy_outputbuses )
    {
      for ( const auto& p : base.outputbuses().buses() )
      {
        circ.outputbuses().add( p.first, p.second );
      }
    }
    if ( settings.copy_statesignals )
    {
      for ( const auto& p : base.statesignals().buses() )
      {
        circ.statesignals().add( p.first, p.second );

        boost::optional<unsigned> initial_value = base.statesignals().initial_value( p.first );
        if ( initial_value )
        {
          circ.statesignals().set_initial_value( p.first, *initial_value );
        }
      }
    }

    if ( settings.copy_modules )
    {
      for ( const auto& p : base.modules() )
      {
        circ.add_module( p.first, p.second );
      }
    }
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
