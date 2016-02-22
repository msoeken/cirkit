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

#include "abc_run_command.hpp"

#include <abc/abc_api.hpp>
#include <abc/functions/cirkit_to_gia.hpp>
#include <abc/functions/gia_to_cirkit.hpp>

namespace cirkit
{

aig_graph abc_run_command_generic( const aig_graph *aig, const std::string& commands )
{
  aig_graph result_aig;

  abc::Abc_Start();
  abc::Abc_Frame_t *abc = abc::Abc_FrameGetGlobalFrame();
  if ( !abc )
  {
    abc::Abc_Stop();
    std::cout << "[e] could not setup up ABC" << std::endl;
    return result_aig;
  }

  /*** load aig into abc ***/
  if ( aig )
  {
    abc::Gia_Man_t *gia = cirkit_to_gia( *aig );
    if ( gia )
    {
      abc::Abc_FrameUpdateGia( abc, gia );
    }
  }

  /*** run command ***/
  auto status = abc::Cmd_CommandExecute( abc, commands.c_str() );

  /*** read gia back to circuit ***/
  abc::Gia_Man_t *result_gia = abc::Abc_FrameGetGia( abc );
  if ( result_gia )
  {
    result_aig = gia_to_cirkit( result_gia );
    abc::Gia_ManStop( result_gia );
  }
  abc::Abc_Stop();

  return result_aig;
}

aig_graph abc_run_command( const std::string& commands )
{
  return abc_run_command_generic( nullptr, commands );
}

aig_graph abc_run_command( const aig_graph& aig, const std::string& commands )
{
  return abc_run_command_generic( &aig, commands );
}

void abc_run_command_no_output( const std::string& commands )
{
  abc_run_command_generic( nullptr, commands );
}

void abc_run_command_no_output( const aig_graph& aig, const std::string& commands )
{
  abc_run_command_generic( &aig, commands );
}

boost::optional< boost::dynamic_bitset<> > abc_run_command_get_counterexample( const std::string& commands )
{
  abc::Abc_Start();
  abc::Abc_Frame_t *abc = abc::Abc_FrameGetGlobalFrame();
  if ( !abc )
  {
    abc::Abc_Stop();
    std::cout << "[e] could not setup up ABC" << std::endl;
    return boost::optional< boost::dynamic_bitset<> >();
  }

  /*** run command ***/
  auto status = abc::Cmd_CommandExecute( abc, commands.c_str() );

  boost::dynamic_bitset<> result;
  abc::Gia_Man_t *result_gia = abc::Abc_FrameGetGia( abc );
  if ( result_gia )
  {
    abc::Abc_Cex_t * cex = abc::Abc_FrameReadCex( abc );
    if ( cex )
    {
      // Abc_CexPrint( cex );

      /*** FFs ***/
      assert( cex->nRegs == 0 && "No support for sequential counterexamples" );

      const unsigned num_inputs = cex->nPis;
      assert( num_inputs > 0u );
      result.resize( num_inputs );

      for ( unsigned i = 0u; i < cex->nPis; ++i )
      {
        if ( abc::Abc_InfoHasBit( cex->pData, cex->nRegs + i ) )
        {
          /* store the inputs in reverse order */
          result[ num_inputs - i - 1u ] = 1;
        }
      }
    }
    abc::Gia_ManStop( result_gia );
  }
  abc::Abc_Stop();

  if ( result.size() > 0u )
  {
    return boost::optional< boost::dynamic_bitset<> >( result );
  }
  else
  {
    return boost::optional< boost::dynamic_bitset<> >();
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
