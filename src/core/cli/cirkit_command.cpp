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

#include "cirkit_command.hpp"

#include <iostream>
#include <memory>

#include <boost/format.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

cirkit_command::cirkit_command( const environment::ptr& env, const std::string& caption, const std::string& publications )
  : command( env, caption, publications )
{
}

bool cirkit_command::run( const std::vector<std::string>& args )
{
  statistics.reset( new properties() );
  return command::run( args );
}

properties::ptr cirkit_command::make_settings() const
{
  auto settings = std::make_shared<properties>();
  if ( opts.find_nothrow( "verbose", false ) )
  {
    settings->set( "verbose", is_verbose() );
  }
  return settings;
}

void cirkit_command::print_runtime( const std::string& key, const std::string& label ) const
{
  if ( label.empty() )
  {
    std::cout << boost::format( "[i] run-time: %.2f secs" ) % statistics->get<double>( key ) << std::endl;
  }
  else
  {
    std::cout << boost::format( "[i] run-time (%s): %.2f secs" ) % label % statistics->get<double>( key ) << std::endl;
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
