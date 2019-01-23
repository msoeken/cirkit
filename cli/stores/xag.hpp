#include <memory>

#include <alice/alice.hpp>
#include <lorina/verilog.hpp>
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/io/write_bench.hpp>
#include <mockturtle/io/write_verilog.hpp>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/views/depth_view.hpp>
#include <mockturtle/views/mapping_view.hpp>

#include <fmt/format.h>

namespace alice
{

using xag_nt = mockturtle::mapping_view<mockturtle::xag_network, true>;
using xag_t = std::shared_ptr<xag_nt>;

ALICE_ADD_STORE( xag_t, "xag", "", "XAG", "XAGs" );

ALICE_DESCRIBE_STORE( xag_t, xag )
{
  return fmt::format( "i/o = {}/{}   gates = {}", xag->num_pis(), xag->num_pos(), xag->num_gates() );
}

ALICE_PRINT_STORE_STATISTICS( xag_t, os, xag )
{
  mockturtle::depth_view depth_xag{*xag};
  os << fmt::format( "XAG   i/o = {}/{}   gates = {}   level = {}", xag->num_pis(), xag->num_pos(), xag->num_gates(), depth_xag.depth() );
  if ( xag->has_mapping() )
  {
    os << fmt::format( "   luts = {}", xag->num_cells() );
  }
  os << "\n";
}

ALICE_LOG_STORE_STATISTICS( xag_t, xag )
{
  mockturtle::depth_view depth_xag{*xag};
  return {
    {"pis", xag->num_pis()},
    {"pos", xag->num_pos()},
    {"gates", xag->num_gates()},
    {"depth", depth_xag.depth()}
  };
}

ALICE_READ_FILE( xag_t, aiger, filename, cmd )
{
  mockturtle::xag_network xag;
  lorina::read_aiger( filename, mockturtle::aiger_reader( xag ) );
  return std::make_shared<xag_nt>( xag );
}

ALICE_WRITE_FILE( xag_t, bench, xag, filename, cmd )
{
  mockturtle::write_bench( *xag, filename );
}

template<>
inline void write<xag_t, io_bench_tag_t>( xag_t const& xag, std::ostream& os, const command& )
{
  mockturtle::write_bench( *xag, os );
}

ALICE_READ_FILE( xag_t, verilog, filename, cmd )
{
  mockturtle::xag_network xag;

  lorina::diagnostic_engine diag;
  if ( lorina::read_verilog( filename, mockturtle::verilog_reader( xag ), &diag ) != lorina::return_code::success )
  {
    std::cout << "[w] parse error\n";
  }
  return std::make_shared<xag_nt>( xag );
}

ALICE_WRITE_FILE( xag_t, verilog, xag, filename, cmd )
{
  mockturtle::write_verilog( *xag, filename );
}

} // namespace alice
