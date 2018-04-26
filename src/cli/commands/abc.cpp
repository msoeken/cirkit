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

#include "abc.hpp"

#include <iostream>

#include <classical/abc/abc_api.hpp>
#include <classical/abc/functions/cirkit_to_gia.hpp>
#include <classical/abc/functions/gia_to_cirkit.hpp>
#include <cli/stores.hpp>

namespace cirkit
{

abc_command::abc_command( const environment::ptr& env )
  : cirkit_command( env, "Runs abc shell with the current aig" )
{
  add_option( "--command,-c", commands, "process semicolon-separated list of commands", true );
  add_flag( "--empty,-e", "don't load current AIG into abc" );
  add_flag( "--nowarning", "don't warn about experimental status" );
  add_new_option();
}

void abc_command::execute()
{
  auto& aigs = env->store<aig_graph>();

  if ( warn_once && !is_set( "nowarning" ) )
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
    extend_if_new( aigs );

    const auto& result_aig = gia_to_cirkit( result_gia );
    aigs.current() = result_aig;

    abc::Gia_ManStop( result_gia );
  }

  abc::Abc_Stop();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
