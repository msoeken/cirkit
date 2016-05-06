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

#include "abc.hpp"

#include <iostream>

#include <core/utils/program_options.hpp>
#include <abc/abc_api.hpp>
#include <abc/functions/cirkit_to_gia.hpp>
#include <abc/functions/gia_to_cirkit.hpp>
#include <classical/cli/stores.hpp>

using namespace boost::program_options;

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

abc_command::abc_command( const environment::ptr& env )
  : cirkit_command( env, "Runs abc shell with the current aig" )
{
  opts.add_options()
    ( "command,c", value_with_default( &commands ), "Process semicolon-separated list of commands" )
    ( "empty,e",                                    "Don't load current AIG into abc" )
    ( "new,n",                                      "Write abc's resulting AIG into a new AIG in the store" )
    ;
}

bool abc_command::execute()
{
  auto& aigs = env->store<aig_graph>();

  if ( warn_once )
  {
    std::cout << "[w] ATTENTION" << std::endl
              << "[w] abc_command is very experimental in its current state" << std::endl
              << "[w] and not expected to work for most scenarios." << std::endl;
    warn_once = false;
  }

  abc::Abc_Start();
  abc::Abc_Frame_t *frame = abc::Abc_FrameGetGlobalFrame();
  if ( !frame )
  {
    abc::Abc_Stop();
    std::cout << "[e] could not setup up ABC" << std::endl;
    return false;
  }

  if ( !aigs.empty() && !is_set( "empty" ) )
  {
    const auto& aig = aigs.current();
    abc::Gia_Man_t *gia = cirkit_to_gia( aig );
    if ( gia )
    {
      abc::Abc_FrameUpdateGia( frame, gia );
    }
  }

  /*** run abc ***/
  if ( commands.empty() )
  {
    /*** interactive mode ***/
    // print the hello line
    abc::Abc_UtilsPrintHello( frame );

    // print history of the recent commands
    abc::Cmd_HistoryPrint( frame, 10 );

    // source the resource file
    abc::Abc_UtilsSource( frame );

    // execute commands given by the user
    char * cmd;
    int status = 0;
    while ( !feof( stdin ) )
    {
      // print command line prompt and
      // get the command from the user
      cmd = abc::Abc_UtilsGetUsersInput( frame );

      // execute the user's command
      status = abc::Cmd_CommandExecute( frame, cmd );

      // stop if the user quitted or an error occurred
      if ( status == -1 || status == -2 )
        break;
    }
  }
  else
  {
    /*** batch mode ***/
    abc::Cmd_CommandExecute( frame, commands.c_str() );
  }

  /*** read gia back to circuit ***/
  abc::Gia_Man_t *result_gia = abc::Abc_FrameGetGia( frame );
  if ( result_gia )
  {
    if ( aigs.empty() || is_set( "new" ) )
    {
      aigs.extend();
    }

    const auto& result_aig = gia_to_cirkit( result_gia );
    aigs.current() = result_aig;

    abc::Gia_ManStop( result_gia );
  }

  abc::Abc_Stop();
  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
