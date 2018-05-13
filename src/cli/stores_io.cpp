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

#include "stores_io.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>

#include <core/io/read_pla.hpp>
#include <core/io/write_pla.hpp>
#include <classical/abc/utils/abc_run_command.hpp>
#include <classical/io/read_aiger.hpp>
#include <classical/io/read_bench.hpp>
#include <classical/io/read_symmetries.hpp>
#include <classical/io/read_unateness.hpp>
#include <classical/io/read_verilog.hpp>
#include <classical/io/write_aiger.hpp>
#include <classical/io/write_bench.hpp>
#include <classical/io/write_verilog.hpp>
#include <classical/mig/mig_verilog.hpp>
#include <classical/xmg/xmg_io.hpp>
#include <classical/xmg/xmg_lut.hpp>

namespace alice
{

using namespace cirkit;

/******************************************************************************
 * bdd_function_t                                                             *
 ******************************************************************************/

template<>
bdd_function_t read<bdd_function_t, io_pla_tag_t>( const std::string& filename, const command& cmd )
{
  return read_pla( filename );
}

template<>
void write<bdd_function_t, io_pla_tag_t>( const bdd_function_t& bdd, const std::string& filename, const command& cmd )
{
  write_pla( bdd, filename );
}

/******************************************************************************
 * aig_graph                                                                  *
 ******************************************************************************/

template<>
bool can_read<aig_graph, io_aiger_tag_t>( command& cmd )
{
  cmd.add_flag( "--nosym", "do not read symmetry file if existing" );
  cmd.add_flag( "--nounate", "do not read unateness file if existing" );
  cmd.add_flag( "--nostrash", "do not strash the AIG when reading (in binary AIGER format)" );
  return true;
}

template<>
aig_graph read<aig_graph, io_aiger_tag_t>( const std::string& filename, const command& cmd )
{
  aig_graph aig;

  try
  {
    if ( boost::ends_with( filename, "aag" ) )
    {
      read_aiger( aig, filename );
    }
    else
    {
      read_aiger_binary( aig, filename, cmd.is_set( "nostrash" ) );
    }
  }
  catch ( const char *e )
  {
    std::cerr << e << std::endl;
    assert( false );
  }

  /* auto-find symmetry file */
  const auto symname = filename.substr( 0, filename.size() - 3 ) + "sym";
  if ( !cmd.is_set( "nosym" ) && std::ifstream( symname.c_str() ).good() )
  {
    /* read symmetries */
    std::cout << "[i] found and read symmetries file" << std::endl;
    read_symmetries( aig, symname );
  }

  /* auto-find unateness file */
  const auto depname = filename.substr( 0, filename.size() - 3 ) + "dep";
  if ( !cmd.is_set( "nounate" ) && std::ifstream( depname.c_str() ).good() )
  {
    /* read unateness */
    std::cout << "[i] found and read unateness dependency file" << std::endl;
    read_unateness( aig, depname );
  }

  return aig;
}

template<>
aig_graph read<aig_graph, io_bench_tag_t>( const std::string& filename, const command& cmd )
{
  aig_graph aig;
  read_bench( aig, filename );
  return aig;
}

template<>
void write<aig_graph, io_aiger_tag_t>( const aig_graph& aig, const std::string& filename, const command& cmd )
{
  if ( boost::ends_with( filename, "aag" ) )
  {
    write_aiger( aig, filename );
  }
  else
  {
    abc_run_command_no_output( aig, boost::str( boost::format( "&w %s") % filename ) );
  }
}

template<>
aig_graph read<aig_graph, io_verilog_tag_t>( const std::string& filename, const command& cmd )
{
  return read_verilog_with_abc( filename );
}

template<>
void write<aig_graph, io_verilog_tag_t>( const aig_graph& aig, const std::string& filename, const command& cmd )
{
  write_verilog( aig, filename );
}

template<>
void write<aig_graph, io_edgelist_tag_t>( const aig_graph& aig, const std::string& filename, const command& cmd )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );

  for ( const auto& e : boost::make_iterator_range( edges( aig ) ) )
  {
    os << source( e, aig ) << " " << target( e, aig ) << std::endl;
  }
}

/******************************************************************************
 * mig_graph                                                                  *
 ******************************************************************************/

template<>
void write<mig_graph, io_verilog_tag_t>( const mig_graph& mig, const std::string& filename, const command& cmd )
{
  write_verilog( mig, filename );
}

template<>
mig_graph read<mig_graph, io_verilog_tag_t>( const std::string& filename, const command& cmd )
{
  return read_mighty_verilog( filename );
}

/******************************************************************************
 * tt                                                                         *
 ******************************************************************************/

template<>
void write<tt, io_pla_tag_t>( const tt& t, const std::string& filename, const command& cmd )
{
  const auto n = tt_num_vars( t );
  std::ofstream out( filename.c_str(), std::ofstream::out );

  out << ".i " << n << std::endl
      << ".o 1" << std::endl;

  auto index = 0u;
  boost::dynamic_bitset<> input( n );

  do {
    if ( t.test( index ) )
    {
      out << input << " 1" << std::endl;
    }

    inc( input );
    ++index;
  } while ( input.any() );

  out << ".e" << std::endl;
}

/******************************************************************************
 * xmg_graph                                                                  *
 ******************************************************************************/

template<>
void write<xmg_graph, io_bench_tag_t>( const xmg_graph& xmg, const std::string& filename, const command& cmd )
{
  if ( !xmg.has_cover() )
  {
    std::cout << "[w] XMG as no cover" << std::endl;
    return;
  }

  auto lut = xmg_to_lut_graph( xmg );
  write_bench( lut, filename );
}

template<>
bool can_read<xmg_graph, io_verilog_tag_t>( command& cmd )
{
  cmd.add_flag( "--as_mig", "read as MIG (translate XOR to MAJ)" );
  cmd.add_flag( "--no_strash", "disable structural hashing when reading the XMG" );
  cmd.add_flag( "--no_invprop", "disable inverter propagation when reading the XMG" );

  return true;
}

template<>
xmg_graph read<xmg_graph, io_verilog_tag_t>( const std::string& filename, const command& cmd )
{
  return read_verilog( filename, !cmd.is_set( "as_mig" ), !cmd.is_set( "no_strash" ), !cmd.is_set( "no_invprop" ) );
}

template<>
bool can_write<xmg_graph, io_verilog_tag_t>( command& cmd )
{
  cmd.add_flag( "--maj_module", "express MAJ gates as modules" );

  return true;
}

template<>
void write<xmg_graph, io_verilog_tag_t>( const xmg_graph& xmg, const std::string& filename, const command& cmd )
{
  auto settings = std::make_shared<properties>();
  settings->set( "maj_module", cmd.is_set( "maj_module" ) );
  write_verilog( xmg, filename, settings );
}

template<>
xmg_graph read<xmg_graph, io_yig_tag_t>( const std::string& filename, const command& cmd )
{
  return xmg_read_yig( filename );
}

template<>
bool can_write<xmg_graph, io_smt_tag_t>( command& cmd )
{
  cmd.add_flag( "--xor_blocks", "write XOR blocks" );

  return true;
}

template<>
void write<xmg_graph, io_smt_tag_t>( const xmg_graph& xmg, const std::string& filename, const command& cmd )
{
  auto settings = std::make_shared<properties>();
  settings->set( "xor_blocks", cmd.is_set( "xor_blocks" ) );
  write_smtlib2( xmg, filename, settings );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
