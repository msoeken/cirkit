/* RevKit (www.revkit.org)
 * Copyright (C) 2009-2014  University of Bremen
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

#include "program_options.hpp"

namespace cirkit
{

  class program_options::priv
  {
  public:
    bool parsed = false;

    boost::program_options::variables_map vm;
    boost::program_options::positional_options_description pod;
  };

  program_options::program_options( unsigned line_length )
    : boost::program_options::options_description( line_length ),
      d( new priv() )
  {
    init();
  }

  program_options::program_options( const std::string& caption, unsigned line_length )
    : boost::program_options::options_description( caption, line_length ),
      d( new priv() )
  {
    init();
  }

  program_options::~program_options()
  {
    delete d;
  }

  bool program_options::good() const
  {
    return !d->vm.count( "help" );
  }

  void program_options::parse( int argc, char ** argv )
  {
    d->pod.add( "filename", 1 );

    boost::program_options::store( boost::program_options::command_line_parser( argc, argv ).options( *this ).positional( d->pod ).run(), d->vm );
    boost::program_options::notify( d->vm );
    d->parsed = true;
  }

  bool program_options::is_set( const std::string& option ) const
  {
    if ( !d->parsed )
    {
      return false;
    }
    return d->vm.count( option ) == 1;
  }

  void program_options::init()
  {
    add_options()( "help,h", "produce help message" );
  }

  bool program_options::parsed() const
  {
    return d->parsed;
  }
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
