#include <memory>

#include <alice/alice.hpp>
#include <lorina/aiger.hpp>
#include <lorina/bench.hpp>
#include <mockturtle/io/aiger_reader.hpp>
#include <mockturtle/io/bench_reader.hpp>
#include <mockturtle/io/write_bench.hpp>
#include <mockturtle/networks/klut.hpp>
#include <mockturtle/views/depth_view.hpp>
#include <mockturtle/views/mapping_view.hpp>

#include <fmt/format.h>

namespace alice
{

using klut_nt = mockturtle::mapping_view<mockturtle::klut_network, true>;
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
  lorina::read_aiger( filename, mockturtle::aiger_reader( klut ) );
  return std::make_shared<klut_nt>( klut );
}

ALICE_READ_FILE( klut_t, bench, filename, cmd )
{
  mockturtle::klut_network klut;
  lorina::read_bench( filename, mockturtle::bench_reader( klut ) );
  return std::make_shared<klut_nt>( klut );
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

} // namespace alice
