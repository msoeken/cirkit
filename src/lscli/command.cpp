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

std::string make_caption( const std::string& caption, const std::string& publications )
{
  std::string c = caption;

  if ( !publications.empty() )
  {
    c += "\n\nBased on the following publication(s):\n" + publications;
  }

  return c;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

command::command( const environment::ptr& env, const std::string& caption, const std::string& publications )
  : env( env ),
    scaption( caption ),
    opts( make_caption( caption, publications ) )
{
  opts.add_options()
    ( "help,h", "produce help message" )
    ;
}

const std::string& command::caption() const
{
  return scaption;
}

bool command::run( const std::vector<std::string>& args )
{
  std::vector<char*> argv( args.size() );
  boost::transform( args, argv.begin(), []( const std::string& s ) { return const_cast<char*>( s.c_str() ); } );
  vm.clear();

  try
  {
    po::store( po::command_line_parser( args.size(), &argv[0] ).options( opts ).positional( pod ).run(), vm );
    po::notify( vm );
  }
  catch ( error& e )
  {
    std::cerr << "[e] " << e.what() << std::endl;
    return false;
  }

  if ( vm.count( "help" ) )
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

  return execute();
}

cli_options command::get_options()
{
  return cli_options( opts, vm, pod );
}

void command::add_positional_option( const std::string& option )
{
  pod.add( option.c_str(), 1 );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
