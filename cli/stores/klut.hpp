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
#include <lorina/bench.hpp>
#include <mockturtle/io/aiger_reader.hpp>
#include <mockturtle/io/bench_reader.hpp>
#include <mockturtle/io/write_bench.hpp>
#include <mockturtle/io/blif_reader.hpp>
#include <mockturtle/io/write_blif.hpp>
#include <mockturtle/networks/klut.hpp>
#include <mockturtle/views/depth_view.hpp>
#include <mockturtle/views/mapping_view.hpp>
#include <mockturtle/views/names_view.hpp>

#include <fmt/format.h>

namespace alice
{

using klut_nt = mockturtle::mapping_view<mockturtle::names_view<mockturtle::klut_network>, true>;
using klut_t = std::shared_ptr<klut_nt>;

ALICE_ADD_STORE( klut_t, "lut", "l", "LUT network", "LUT networks" );

ALICE_DESCRIBE_STORE( klut_t, klut )
{
  return fmt::format( "i/o = {}/{}   gates = {}", klut->num_pis(), klut->num_pos(), klut->num_gates() );
}

ALICE_PRINT_STORE_STATISTICS( klut_t, os, klut )
{
  mockturtle::depth_view depth_klut{*klut};
  os << fmt::format( "LUT network   i/o = {}/{}   gates = {}   level = {}", klut->num_pis(), klut->num_pos(), klut->num_gates(), depth_klut.depth() );
  if ( klut->has_mapping() )
  {
    os << fmt::format( "   luts = {}", klut->num_cells() );
  }
  os << "\n";
}

ALICE_LOG_STORE_STATISTICS( klut_t, klut )
{
  mockturtle::depth_view depth_klut{*klut};
  return {
    {"pis", klut->num_pis()},
    {"pos", klut->num_pos()},
    {"gates", klut->num_gates()},
    {"depth", depth_klut.depth()}
  };
}

ALICE_READ_FILE( klut_t, aiger, filename, cmd )
{
  mockturtle::klut_network klut;
  mockturtle::names_view<mockturtle::klut_network> named_klut( klut );

  lorina::diagnostic_engine diag;
  if ( lorina::read_aiger( filename, mockturtle::aiger_reader( named_klut ), &diag ) != lorina::return_code::success )
  {
    std::cout << "[w] parse error\n";
  }
  return std::make_shared<klut_nt>( named_klut );
}

ALICE_READ_FILE( klut_t, bench, filename, cmd )
{
  mockturtle::klut_network klut;
  mockturtle::names_view<mockturtle::klut_network> named_klut( klut );

  lorina::diagnostic_engine diag;
  if ( lorina::read_bench( filename, mockturtle::bench_reader( named_klut ), &diag ) != lorina::return_code::success )
  {
    std::cout << "[w] parse error\n";
  }
  return std::make_shared<klut_nt>( named_klut );
}

ALICE_WRITE_FILE( klut_t, bench, klut, filename, cmd )
{
  mockturtle::write_bench( *klut, filename );
}

template<>
inline void write<klut_t, io_bench_tag_t>( klut_t const& klut, std::ostream& os, const command& )
{
  mockturtle::write_bench( *klut, os );
}

ALICE_READ_FILE( klut_t, blif, filename, cmd )
{
  mockturtle::klut_network klut;
  mockturtle::names_view<mockturtle::klut_network> named_klut( klut );

  lorina::diagnostic_engine diag;
  if ( lorina::read_blif( filename, mockturtle::blif_reader( named_klut ), &diag ) != lorina::return_code::success )
  {
    std::cout << "[w] parse error\n";
  }
  return std::make_shared<klut_nt>( named_klut );
}

ALICE_WRITE_FILE( klut_t, blif, klut, filename, cmd )
{
  mockturtle::write_blif( *klut, filename );
}

} // namespace alice
