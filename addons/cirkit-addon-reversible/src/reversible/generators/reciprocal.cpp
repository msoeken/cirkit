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

#include "reciprocal.hpp"

#include <cmath>
#include <fstream>
#include <iostream>

#include <boost/format.hpp>

#include <core/io/read_pla.hpp>
#include <core/utils/bdd_utils.hpp>
#include <core/utils/bitset_utils.hpp>
#include <core/utils/system_utils.hpp>
#include <core/utils/timer.hpp>
#include <classical/abc/abc_api.hpp>
#include <classical/abc/abc_manager.hpp>
#include <classical/optimization/optimization.hpp>
#include <classical/optimization/exorcism_minimization.hpp>
#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/synthesis/embed_bdd.hpp>
#include <reversible/synthesis/esop_synthesis.hpp>
#include <reversible/synthesis/qmdd_synthesis.hpp>
#include <reversible/synthesis/rcbdd_synthesis.hpp>
#include <reversible/synthesis/symbolic_transformation_based_synthesis.hpp>

#define LN(x) if ( verbose ) { std::cout << x << std::endl; }

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

class generate_reciprocal_manager
{
public:
  generate_reciprocal_manager( unsigned bitwidth, const properties::ptr& settings )
    : bitwidth( bitwidth )
  {
    /* settings */
    method       = get( settings, "method",       0u );
    verilog_name = get( settings, "verilog_name", std::string( "/tmp/test.v" ) );
    esop_name    = get( settings, "esop_name",    std::string( "/tmp/test.esop" ) );
    pla_name     = get( settings, "pla_name",     std::string( "/tmp/test.pla" ) );
    only_write   = get( settings, "only_write",   false );
    verbose      = get( settings, "verbose",      false );
  }

  circuit run()
  {
    write_verilog();

    if ( only_write )
    {
      return circuit();
    }

    switch ( method )
    {
    default:
      assert( false );
    case 0u:
      return synthesize_with_esop();
    case 1u:
      return synthesize_with_pla();
    }
  }

protected:
  virtual void write_verilog() const = 0;

  virtual circuit synthesize_with_esop() = 0;
  virtual circuit synthesize_with_pla();

protected:
  unsigned bitwidth;

  unsigned    method; /* 0: esop, 1: pla */
  bool        only_write;
  bool        verbose;
  std::string verilog_name;
  std::string esop_name;
  std::string pla_name;
};

circuit generate_reciprocal_manager::synthesize_with_pla()
{
  circuit circ;
  rcbdd cf;

  LN( "[i] read PLA" );
  auto bdd = read_pla( pla_name );
  LN( "[i] embed BDD" );
  embed_bdd( cf, bdd );
  LN( "[i] synthesize RCBDD" );

  auto syn_settings = std::make_shared<properties>();
  auto syn_statistics = std::make_shared<properties>();
  auto esopmin_settings = std::make_shared<properties>();
  syn_settings->set( "esopmin", dd_based_exorcism_minimization_func( esopmin_settings ) );
  syn_settings->set( "verbose", verbose );
  syn_settings->set( "cnf_from_aig", true );
  symbolic_transformation_based_synthesis_sat( circ, cf, syn_settings, syn_statistics );

  std::cout << boost::format( "runtime (SAT):   %.2f\nruntime (total): %.2f\n" ) % syn_statistics->get<double>( "runtime" ) % syn_statistics->get<double>( "solving_time" ) << std::endl;

  return circ;
}

class generate_reciprocal_direct_manager : public generate_reciprocal_manager
{
public:
  generate_reciprocal_direct_manager( unsigned bitwidth, const properties::ptr& settings )
    : generate_reciprocal_manager( bitwidth, settings )
  {
  }

protected:
  void write_verilog() const;

  circuit synthesize_with_esop();
  circuit synthesize_with_pla();
};

class generate_reciprocal_newton_manager : public generate_reciprocal_manager
{
public:
  generate_reciprocal_newton_manager( unsigned bitwidth, const properties::ptr& settings )
    : generate_reciprocal_manager( bitwidth, settings )
  {
    blif_name  = get( settings, "blif_name",  std::string( "/tmp/test.blif" ) );
    iterations = get( settings, "iterations", log2( ( bitwidth + 1 ) / log2( 17.0 ) ) );
  }

protected:
  void write_verilog() const;

  circuit synthesize_with_esop();
  circuit synthesize_with_pla();

protected:
  std::string blif_name;
  unsigned    iterations;
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void generate_reciprocal_direct_manager::write_verilog() const
{
  using boost::format;

  std::ofstream os( verilog_name.c_str(), std::ofstream::out );

  os << "module top( a, z );" << std::endl
     << format( "  input [%d:0] a;" ) % ( bitwidth - 1 ) << std::endl
     << format( "  output [%d:0] z;" ) % ( bitwidth - 1 ) << std::endl << std::endl
     << format( "  wire [%d:0] n = %d'b1%s;" ) % bitwidth % ( bitwidth + 1u ) % std::string( bitwidth, '0' ) << std::endl
     << format( "  wire [%d:0] d = {1'b0, a};" ) % bitwidth << std::endl
     << format( "  wire [%d:0] t; " ) % bitwidth << std::endl << std::endl
     << "  assign t = n / d;" << std::endl
     << format( "  assign z = t[%d:0];" ) % ( bitwidth - 1 ) << std::endl
     << "endmodule" << std::endl;

  os.close();
}

circuit generate_reciprocal_direct_manager::synthesize_with_esop()
{
  abc_manager::get()->run_command_no_output( boost::str( boost::format( "%%read %s; %%blast; &put; satclp; sop; fx; strash; dc2; &get; &exorcism %s" ) % verilog_name % esop_name ) );

  circuit circ;
  esop_synthesis( circ, esop_name );
  return circ;
}

circuit generate_reciprocal_direct_manager::synthesize_with_pla()
{
  LN( "[i] generate PLA" );
  abc_manager::get()->run_command_no_output( boost::str( boost::format( "%%read %s; %%blast; &dc2; &dc2; &dc2; &dc2; &dc2; &put; collapse; write %s" ) % verilog_name % pla_name ) );

  return generate_reciprocal_manager::synthesize_with_pla();
}

void generate_reciprocal_newton_manager::write_verilog() const
{
  using boost::format;

  std::ofstream os( verilog_name.c_str(), std::ofstream::out );

  os << "module reciprocal_newton( d, y );" << std::endl
     << format( "  input [%d:0] d;" ) % ( bitwidth - 1 ) << std::endl
     << format( "  output [%d:0] y;" ) % ( bitwidth - 1 ) << std::endl << std::endl;

  boost::dynamic_bitset<> bv1( 2 * bitwidth, static_cast<unsigned>( pow( 2.0, 2 * bitwidth ) * 14.0 / 17.0 ) );
  boost::dynamic_bitset<> bv2( bitwidth, static_cast<unsigned>( pow( 2.0, bitwidth ) * 15.0 / 17.0 ) );

  os << format( "  wire signed [%d:0] dp;" ) % ( bitwidth + 2 ) << std::endl
     << format( "  wire signed [%d:0] v1  = %d'b010%s; /* 48/17 */" ) % ( 2 * bitwidth + 2 ) % ( 2 * bitwidth + 3 ) % to_string( bv1 ) << std::endl
     << format( "  wire signed [%d:0] v2  = %d'b001%s; /* 32/17 */" ) % ( bitwidth + 2 ) % ( bitwidth + 3 ) % to_string( bv2 ) << std::endl
     << format( "  wire signed [%d:0] one = %d'b001%s; /* 1 */" ) % ( 2 * bitwidth + 2 ) % ( 2 * bitwidth + 3 ) % std::string( 2 * bitwidth, '0' ) << std::endl
     << format( "  wire signed [%d:0] w1;" ) % ( 2 * bitwidth + 5 ) << std::endl
     << format( "  wire signed [%d:0] w2;" ) % ( 2 * bitwidth + 2 ) << std::endl
     << format( "  wire signed [%d:0] x [%d:0];" ) % ( 2 * bitwidth + 2 ) % iterations << std::endl
     << format( "  wire signed [%d:0] w4 [%d:1];" ) % ( 3 * bitwidth + 5 ) % iterations << std::endl
     << format( "  wire signed [%d:0] w5 [%d:1];" ) % ( 2 * bitwidth + 2 ) % iterations << std::endl
     << format( "  wire signed [%d:0] w6 [%d:1];" ) % ( 4 * bitwidth + 5 ) % iterations << std::endl << std::endl;

  os << format( "  assign dp = (d[%d] == 1'b1) ? {3'b000, d} :" ) % ( bitwidth - 1 ) << std::endl;
  for ( auto i = bitwidth - 2; i >= 1; --i )
  {
    os << format( "              (d[%d] == 1'b1) ? {3'b000, d[%d:0], %d'b%s} :" ) % i % i % ( bitwidth - 1 - i ) % std::string( bitwidth - 1 - i, '0' ) << std::endl;
  }
  os << format( "              (d[0] == 1'b1) ? {3'b000, d[0], %d'b%s} : %d'b%s;" ) % ( bitwidth - 1 ) % std::string( bitwidth - 1, '0' ) % ( bitwidth + 3 ) % std::string( bitwidth +3, '0' ) << std::endl << std::endl;

  os << "  assign w1 = v2 * dp;" << std::endl
     << format( "  assign w2 = w1[%d:0];" ) % ( 2 * bitwidth + 2 ) << std::endl
     << "  assign x[0] = v1 - w2;" << std::endl << std::endl;

  os << "  genvar i;" << std::endl
     << "  generate" << std::endl
     << format( "    for ( i = 1; i <= %d; i = i + 1 )" ) % iterations << std::endl
     << "      begin" << std::endl
     << "        assign w4[i] = dp * x[i - 1];" << std::endl
     << format( "        assign w5[i] = one - w4[i][%d:%d];" ) % ( 3 * bitwidth + 2 ) % bitwidth << std::endl
     << "        assign w6[i] = x[i - 1] * w5[i];" << std::endl
     << format( "        assign x[i] = x[i - 1] + w6[i][%d:%d];" ) % ( 4 * bitwidth + 2 ) % ( 2 * bitwidth ) << std::endl
     << "      end" << std::endl
     << "  endgenerate" << std::endl << std::endl;

  for ( auto i = bitwidth - 1; i >= 3; --i )
  {
    if ( i == ( bitwidth - 1 ) )
    {
      os << "  assign y = ";
    }
    else
    {
      os << "             ";
    }

    os << format( "(d[%d] == 1'b1) ? {%d'b%s, x[%d][%d:%d]} :" ) % i % ( i - 2 ) % std::string( i - 2, '0' ) % iterations % ( 2 * bitwidth + 2 ) % ( bitwidth + 1 + i ) << std::endl;
  }
  os << format( "             (d[2] == 1'b1) ? x[%d][%d:%d] :" ) % iterations % ( 2 * bitwidth + 2 ) % ( bitwidth + 3 ) << std::endl;
  os << format( "             (d[1] == 1'b1) ? x[%d][%d:%d] :" ) % iterations % ( 2 * bitwidth + 1 ) % ( bitwidth + 2 ) << std::endl;
  os << format( "             (d[0] == 1'b1) ? x[%d][%d:%d] : %d'b%s;" ) % iterations % ( 2 * bitwidth ) % ( bitwidth + 1 ) % bitwidth % std::string( bitwidth, '0' ) << std::endl;
  os << "endmodule" << std::endl;

  os.close();
}

circuit generate_reciprocal_newton_manager::synthesize_with_esop()
{
  LN( "[i] generate BLIF" );
  execute_and_omit( boost::str( boost::format( "yosys -p \"read_verilog %s; proc; opt; techmap; opt; stat; write_blif %s\"" ) % verilog_name % blif_name ) );
  LN( "[i] generate ESOP" );
  abc_manager::get()->run_command_no_output( boost::str( boost::format( "read %s; satclp; sop; fx; strash; dc2; &get; &exorcism %s" ) % blif_name % esop_name ) );

  circuit circ;
  esop_synthesis( circ, esop_name );
  return circ;
}

circuit generate_reciprocal_newton_manager::synthesize_with_pla()
{
  LN( "[i] generate BLIF" );
  execute_and_omit( boost::str( boost::format( "yosys -p \"read_verilog %s; proc; opt; techmap; opt; stat; write_blif %s\"" ) % verilog_name % blif_name ) );

  LN( "[i] generate PLA" );
  abc_manager::get()->run_command_no_output( boost::str( boost::format( "read %s; strash; &get; &dc2; &dc2; &dc2; &dc2; &dc2; &put; collapse; write %s" ) % blif_name % pla_name ) );

  return generate_reciprocal_manager::synthesize_with_pla();
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

circuit generate_reciprocal_direct( unsigned bitwidth, const properties::ptr& settings, const properties::ptr& statistics )
{
  /* timing */
  properties_timer t( statistics );

  generate_reciprocal_direct_manager mgr( bitwidth, settings );
  return mgr.run();
}

circuit generate_reciprocal_newton( unsigned bitwidth, const properties::ptr& settings, const properties::ptr& statistics )
{
  /* timing */
  properties_timer t( statistics );

  generate_reciprocal_newton_manager mgr( bitwidth, settings );
  return mgr.run();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
