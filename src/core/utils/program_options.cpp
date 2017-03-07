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

#include "program_options.hpp"

namespace cirkit
{

  class program_options::priv
  {
  public:
    bool parsed = false;
    std::string positional_option = "filename";

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
    d->pod.add( d->positional_option.c_str(), 1 );

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

  void program_options::clear()
  {
    d->vm.clear();
  }

  void program_options::set_positional_option( const std::string& name )
  {
    d->positional_option = name;
  }

  const boost::program_options::variables_map& program_options::variables() const
  {
    return d->vm;
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
