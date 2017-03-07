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

#include "flatten_circuit.hpp"

#include "../circuit.hpp"
#include "../target_tags.hpp"

#include "copy_metadata.hpp"

#include <boost/range/algorithm.hpp>

namespace cirkit
{

  void flatten_circuit( const circuit& base, circuit& circ, bool keep_meta_data )
  {
    copy_metadata_settings settings;
    settings.copy_modules = keep_meta_data;
    settings.copy_inputbuses = keep_meta_data;
    settings.copy_outputbuses = keep_meta_data;
    settings.copy_statesignals = keep_meta_data;
    copy_metadata( base, circ, settings );

    for ( const auto& g : base )
    {
      if ( is_module( g ) )
      {
        circuit flattened;
        const module_tag& tag = boost::any_cast<module_tag>( g.type() );
        flatten_circuit( *tag.reference.get(), flattened );

        for ( const auto& fg : flattened )
        {
          gate& new_gate = circ.append_gate();

          boost::for_each( g.controls(), [&new_gate]( variable c ) { new_gate.add_control( c ); } );

          // TODO write test case
          for ( const auto& v : fg.controls() )
          {
            new_gate.add_control( make_var( g.targets().at( v.line() ), v.polarity() ) );
          }

          for ( const auto& l : fg.targets() )
          {
            new_gate.add_target( g.targets().at( l ) );
          }

          new_gate.set_type( fg.type() );
        }
      }
      else
      {
        circ.append_gate() = g;
      }
    }
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
