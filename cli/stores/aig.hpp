/* CirKit: A circuit toolkit
 * Copyright (C) 2017-2019  EPFL
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

#include <memory>

#include <alice/alice.hpp>
#include <lorina/aiger.hpp>
#include <lorina/blif.hpp>
#include <lorina/verilog.hpp>
#include <mockturtle/io/aiger_reader.hpp>
#include <mockturtle/io/blif_reader.hpp>
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/io/write_bench.hpp>
#include <mockturtle/io/write_blif.hpp>
#include <mockturtle/io/write_verilog.hpp>
#include <mockturtle/networks/aig.hpp>
#include <mockturtle/views/depth_view.hpp>
#include <mockturtle/views/mapping_view.hpp>
#include <mockturtle/views/names_view.hpp>

#include <fmt/format.h>

namespace alice
{

using aig_nt = mockturtle::mapping_view<mockturtle::names_view<mockturtle::aig_network>, true>;
using aig_t = std::shared_ptr<aig_nt>;

ALICE_ADD_STORE( aig_t, "aig", "a", "AIG", "AIGs" );

ALICE_DESCRIBE_STORE( aig_t, aig )
{
  return fmt::format( "i/o = {}/{}   gates = {}", aig->num_pis(), aig->num_pos(), aig->num_gates() );
}

ALICE_PRINT_STORE_STATISTICS( aig_t, os, aig )
{
  mockturtle::depth_view depth_aig{*aig};
  os << fmt::format( "AIG   i/o = {}/{}   gates = {}   level = {}", aig->num_pis(), aig->num_pos(), aig->num_gates(), depth_aig.depth() );
  if ( aig->has_mapping() )
  {
    os << fmt::format( "   luts = {}", aig->num_cells() );
  }
  os << "\n";
}

ALICE_LOG_STORE_STATISTICS( aig_t, aig )
{
  mockturtle::depth_view depth_aig{*aig};
  return {
    {"pis", aig->num_pis()},
    {"pos", aig->num_pos()},
    {"gates", aig->num_gates()},
    {"depth", depth_aig.depth()}
  };
}

ALICE_READ_FILE( aig_t, aiger, filename, cmd )
{
  mockturtle::aig_network aig;

  lorina::diagnostic_engine diag;
  if ( lorina::read_aiger( filename, mockturtle::aiger_reader( aig ), &diag ) != lorina::return_code::success )
  {
    std::cout << "[w] parse error\n";
  }
  return std::make_shared<aig_nt>( aig );
}

ALICE_WRITE_FILE( aig_t, bench, aig, filename, cmd )
{
  mockturtle::write_bench( *aig, filename );
}

ALICE_READ_FILE( aig_t, verilog, filename, cmd )
{
  mockturtle::aig_network aig;

  lorina::diagnostic_engine diag;
  if ( lorina::read_verilog( filename, mockturtle::verilog_reader( aig ), &diag ) != lorina::return_code::success )
  {
    std::cout << "[w] parse error\n";
  }
  return std::make_shared<aig_nt>( aig );
}

ALICE_WRITE_FILE( aig_t, verilog, aig, filename, cmd )
{
  mockturtle::write_verilog( *aig, filename );
}

ALICE_WRITE_FILE( aig_t, blif, aig, filename, cmd )
{
  mockturtle::write_blif( *aig, filename );
}

} // namespace alice
