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
