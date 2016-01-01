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

#include "utils.hpp"

#include <cstdlib>

#include <core/utils/system_utils.hpp>

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

bool read_command_line( const std::string& prefix, std::string& line )
{
#ifdef USE_READLINE
  auto * cline = readline( boost::str( boost::format( "%s> " ) % prefix ).c_str() );

  /* something went wrong? */
  if ( !cline )
  {
    return false;
  }

  line = cline;
  boost::trim( line );
  free( cline );

  return true;

#else // USE_READLINE

  std::cout << prefix << "> ";
  std::flush(std::cout);
  if( !getline( std::cin, line ) ) {
    return false;
  }

  boost::trim( line );
  return true;
#endif // USE_READLINE
}

bool execute_line( const environment::ptr& env, const std::string& line, const std::map<std::string, std::shared_ptr<command>>& commands )
{
  /* ignore comments and empty lines */
  if ( line.empty() || line[0] == '#' ) { return false; }

  /* escape to shell */
  if ( line[0] == '!' )
  {
    const auto now = std::chrono::system_clock::now();
    const auto result = execute_and_return_tee( line.substr( 1u ) );

    if ( env->log )
    {
      command::log_map_t log;
      log["status"] = result.first;
      log["output"] = result.second;
      env->log_command( command::log_opt_t( log ), line, now );
    }

    return true;
  }

  auto vline = split_with_quotes( line );

  const auto it = commands.find( vline.front() );
  if ( it != commands.end() )
  {
    const auto now = std::chrono::system_clock::now();
    const auto result = it->second->run( vline );

    if ( result && env->log )
    {
      env->log_command( it->second, line, now );
    }

    return result;
  }
  else
  {
    std::cout << "unknown command: " << vline.front() << std::endl;
    return false;
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
