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
#include <lorina/verilog.hpp>
#include <mockturtle/io/aiger_reader.hpp>
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/io/write_bench.hpp>
#include <mockturtle/io/write_verilog.hpp>
#include <mockturtle/networks/mig.hpp>
#include <mockturtle/views/depth_view.hpp>
#include <mockturtle/views/mapping_view.hpp>
#include <mockturtle/views/names_view.hpp>

#include <fmt/format.h>

namespace alice
{

using mig_nt = mockturtle::mapping_view<mockturtle::names_view<mockturtle::mig_network>, true>;
using mig_t = std::shared_ptr<mig_nt>;

ALICE_ADD_STORE( mig_t, "mig", "m", "MIG", "MIGs" );

ALICE_DESCRIBE_STORE( mig_t, mig )
{
  return fmt::format( "i/o = {}/{}   gates = {}", mig->num_pis(), mig->num_pos(), mig->num_gates() );
}

ALICE_PRINT_STORE_STATISTICS( mig_t, os, mig )
{
  mockturtle::depth_view depth_mig{*mig};
  os << fmt::format( "MIG   i/o = {}/{}   gates = {}   level = {}", mig->num_pis(), mig->num_pos(), mig->num_gates(), depth_mig.depth() );
  if ( mig->has_mapping() )
  {
    os << fmt::format( "   luts = {}", mig->num_cells() );
  }
  os << "\n";
}

ALICE_LOG_STORE_STATISTICS( mig_t, mig )
{
  mockturtle::depth_view depth_mig{*mig};
  return {
    {"pis", mig->num_pis()},
    {"pos", mig->num_pos()},
    {"gates", mig->num_gates()},
    {"depth", depth_mig.depth()}
  };
}

ALICE_READ_FILE( mig_t, aiger, filename, cmd )
{
  mockturtle::mig_network mig;

  lorina::diagnostic_engine diag;
  if ( lorina::read_aiger( filename, mockturtle::aiger_reader( mig ), &diag ) != lorina::return_code::success )
  {
    std::cout << "[w] parse error\n";
  }
  return std::make_shared<mig_nt>( mig );
}

ALICE_WRITE_FILE( mig_t, bench, mig, filename, cmd )
{
  mockturtle::write_bench( *mig, filename );
}

template<>
inline void write<mig_t, io_bench_tag_t>( mig_t const& mig, std::ostream& os, const command& )
{
  mockturtle::write_bench( *mig, os );
}

ALICE_READ_FILE( mig_t, verilog, filename, cmd )
{
  mockturtle::mig_network mig;

  lorina::diagnostic_engine diag;
  if ( lorina::read_verilog( filename, mockturtle::verilog_reader( mig ), &diag ) != lorina::return_code::success )
  {
    std::cout << "[w] parse error\n";
  }
  return std::make_shared<mig_nt>( mig );
}

ALICE_WRITE_FILE( mig_t, verilog, mig, filename, cmd )
{
  mockturtle::write_verilog( *mig, filename );
}

ALICE_WRITE_FILE( mig_t, blif, mig, filename, cmd )
{
  mockturtle::write_blif( *mig, filename );
}

} // namespace alice
