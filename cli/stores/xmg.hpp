#include <memory>

#include <alice/alice.hpp>
#include <lorina/aiger.hpp>
#include <lorina/verilog.hpp>
#include <mockturtle/io/aiger_reader.hpp>
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/io/write_bench.hpp>
#include <mockturtle/io/write_verilog.hpp>
#include <mockturtle/networks/xmg.hpp>
#include <mockturtle/views/depth_view.hpp>
#include <mockturtle/views/mapping_view.hpp>

#include <fmt/format.h>

namespace alice
{

using xmg_nt = mockturtle::mapping_view<mockturtle::xmg_network, true>;
using xmg_t = std::shared_ptr<xmg_nt>;

ALICE_ADD_STORE( xmg_t, "xmg", "x", "XMG", "XMGs" );

ALICE_DESCRIBE_STORE( xmg_t, xmg )
{
  return fmt::format( "i/o = {}/{}   gates = {}", xmg->num_pis(), xmg->num_pos(), xmg->num_gates() );
}

ALICE_PRINT_STORE_STATISTICS( xmg_t, os, xmg )
{
  mockturtle::depth_view depth_xmg{*xmg};
  os << fmt::format( "XMG   i/o = {}/{}   gates = {}   level = {}", xmg->num_pis(), xmg->num_pos(), xmg->num_gates(), depth_xmg.depth() );
  if ( xmg->has_mapping() )
  {
    os << fmt::format( "   luts = {}", xmg->num_cells() );
  }
  os << "\n";
}

ALICE_LOG_STORE_STATISTICS( xmg_t, xmg )
{
  mockturtle::depth_view depth_xmg{*xmg};
  return {
    {"pis", xmg->num_pis()},
    {"pos", xmg->num_pos()},
    {"gates", xmg->num_gates()},
    {"depth", depth_xmg.depth()}
  };
}

ALICE_READ_FILE( xmg_t, aiger, filename, cmd )
{
  mockturtle::xmg_network xmg;

  lorina::diagnostic_engine diag;
  if ( lorina::read_aiger( filename, mockturtle::aiger_reader( xmg ), &diag ) != lorina::return_code::success )
  {
    std::cout << "[w] parse error\n";
  }
  return std::make_shared<xmg_nt>( xmg );
}

ALICE_WRITE_FILE( xmg_t, bench, xmg, filename, cmd )
{
  mockturtle::write_bench( *xmg, filename );
}

template<>
inline void write<xmg_t, io_bench_tag_t>( xmg_t const& xmg, std::ostream& os, const command& )
{
  mockturtle::write_bench( *xmg, os );
}

ALICE_READ_FILE( xmg_t, verilog, filename, cmd )
{
  mockturtle::xmg_network xmg;

  lorina::diagnostic_engine diag;
  if ( lorina::read_verilog( filename, mockturtle::verilog_reader( xmg ), &diag ) != lorina::return_code::success )
  {
    std::cout << "[w] parse error\n";
  }
  return std::make_shared<xmg_nt>( xmg );
}

ALICE_WRITE_FILE( xmg_t, verilog, xmg, filename, cmd )
{
  mockturtle::write_verilog( *xmg, filename );
}

} // namespace alice
