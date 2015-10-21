/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

#include "command.hpp"

#include <boost/range/algorithm.hpp>

namespace cirkit
{

using namespace boost::program_options;

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

command::command( const environment::ptr& env, const std::string& caption )
  : env( env ),
    scaption( caption ),
    opts( caption )
{
}

const std::string& command::caption() const
{
  return scaption;
}

bool command::run( const std::vector<std::string>& args )
{
  std::vector<char*> argv( args.size() );
  boost::transform( args, argv.begin(), []( const std::string& s ) { return const_cast<char*>( s.c_str() ); } );
  opts.clear();

  try
  {
    opts.parse( args.size(), &argv[0] );
  }
  catch ( error& e )
  {
    std::cerr << "[e] " << e.what() << std::endl;
    return false;
  }

  if ( !opts.good() )
  {
    std::cout << opts << std::endl;
    return false;
  }

  for ( const auto& p : validity_rules() )
  {
    if ( !p.first() )
    {
      std::cerr << "[e] " << p.second << std::endl;
      return false;
    }
  }

  statistics.reset( new properties() );
  return execute();
}

properties::ptr command::make_settings() const
{
  auto settings = std::make_shared<properties>();
  if ( opts.find_nothrow( "verbose", false ) )
  {
    settings->set( "verbose", is_verbose() );
  }
  return settings;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
