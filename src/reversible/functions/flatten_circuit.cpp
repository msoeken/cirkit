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
// End:
