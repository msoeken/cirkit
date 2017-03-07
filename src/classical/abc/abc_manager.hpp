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

/**
 * @file abc_manager.hpp
 *
 * @brief ABC interface
 *
 * Interface to ABC
 *
 * @author Heinz Riener
 * @since  2.3
 */

#ifndef ABC_MANAGER_HPP
#define ABC_MANAGER_HPP

#include <classical/abc/abc_api.hpp>
#include <classical/abc/functions/gia_to_cirkit.hpp>
#include <classical/abc/functions/cirkit_to_gia.hpp>

#include <boost/dynamic_bitset.hpp>
#include <boost/format.hpp>

#include <iostream>
#include <string>

namespace cirkit
{
using abc_counterexample_t = boost::dynamic_bitset<>;
using abc_counterexample_opt_t = boost::optional< abc_counterexample_t >;

class abc_manager
{
public:
  static abc_manager *get()
  {
    if ( instance )
    {
      instance = new abc_manager();
    }
    return instance;
  }

  virtual ~abc_manager()
  {
    abc::Abc_Stop();
  }

  std::pair< int, aig_graph > run_command( const std::string& command )
  {
    return run_command_generic( command );
  }

  std::pair< int, aig_graph > run_command( const aig_graph& aig, const std::string& command )
  {
    return run_command_generic( command, &aig );
  }

  int run_command_no_output( const std::string& command )
  {
    return run_command_generic( command ).first;
  }

  int run_command_no_output( const aig_graph& aig, const std::string& command )
  {
    return run_command_generic( command, &aig ).first;
  }

  int write_aig( const aig_graph& aig, const std::string& filename )
  {
    return run_command_no_output( aig, ( boost::format( "&w %s;" ) % filename ).str() );
  }

  std::pair< int, abc_counterexample_opt_t > cec( const aig_graph& circuit, const aig_graph& spec )
  {
    const std::string circuit_filename = ( boost::format( "/tmp/circuit-%s.aig" ) % getpid() ).str();
    const std::string spec_filename = ( boost::format( "/tmp/spec-%s.aig" ) % getpid() ).str();
    write_aig( circuit, circuit_filename );
    write_aig( spec, spec_filename );

    /*** note that the assignment only contains the inputs ***/
    const constexpr auto command = "&r %s; &miter %s; &cec -s -m";
    return run_command_get_counterexample( ( boost::format( command ) % circuit_filename % spec_filename ).str() );
  }

private:
  abc_manager()
  {
    abc::Abc_Start();
  }

  abc::Abc_Frame_t *frame()
  {
    abc::Abc_Frame_t* abc = abc::Abc_FrameGetGlobalFrame();
    if ( !abc )
    {
      std::cerr << "[e] could not setup up ABC" << std::endl;
      return nullptr;
    }
    return abc;
  }

  std::pair< int, aig_graph > run_command_generic( const std::string& command, const aig_graph *aig = nullptr )
  {
    aig_graph result_aig;
    auto abc = frame();
    if ( !abc )
    {
      return { -1, result_aig };
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
    const int status = abc::Cmd_CommandExecute( abc, command.c_str() );

    /*** read gia back to circuit ***/
    abc::Gia_Man_t* result_gia = abc::Abc_FrameGetGia( abc );
    if ( result_gia )
    {
      result_aig = gia_to_cirkit( result_gia );
      abc::Gia_ManStop( result_gia );
    }

    return { status, result_aig };
  }

  std::pair< int, abc_counterexample_opt_t > run_command_get_counterexample( const std::string& commands, const aig_graph *aig = nullptr )
  {
    abc_counterexample_t counterexample;
    auto abc = frame();
    if ( !abc )
    {
      return { -1, counterexample };
    }

    /*** run command ***/
    auto status = abc::Cmd_CommandExecute( abc, commands.c_str() );

    abc::Gia_Man_t* result_gia = abc::Abc_FrameGetGia( abc );
    if ( result_gia )
    {
      auto cex = static_cast<abc::Abc_Cex_t*>( abc::Abc_FrameReadCex( abc ) );
      if ( cex )
      {
        // Abc_CexPrint( cex );

        /*** FFs ***/
        assert( cex->nRegs == 0 && "No support for sequential counterexamples" );

        const unsigned num_inputs = cex->nPis;
        assert( num_inputs > 0u );
        counterexample.resize( num_inputs );

        for ( auto i = 0; i < cex->nPis; ++i )
        {
          if ( abc::Abc_InfoHasBit( cex->pData, cex->nRegs + i ) )
          {
            /* store the inputs in reverse order */
            counterexample[num_inputs - i - 1u] = 1;
          }
        }
      }
      abc::Gia_ManStop( result_gia );
    }

    const auto cex_opt = counterexample.size() ? abc_counterexample_opt_t( counterexample ) : abc_counterexample_opt_t();
    return { status, cex_opt };
  }

  abc_manager( const abc_manager& );
  abc_manager& operator=( const abc_manager& );

  static abc_manager *instance;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
