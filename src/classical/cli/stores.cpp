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

#include "stores.hpp"

#include <fstream>
#include <sstream>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/range/iterator_range.hpp>
#include <range/v3/algorithm/transform.hpp>

#include <core/graph/depth.hpp>
#include <core/utils/bitset_utils.hpp>
#include <core/utils/range_utils.hpp>

#include <classical/abc/utils/abc_run_command.hpp>
#include <classical/functions/aig_from_truth_table.hpp>
#include <classical/functions/aig_to_mig.hpp>
#include <classical/functions/compute_levels.hpp>
#include <classical/functions/simulate_aig.hpp>
#include <classical/io/read_aiger.hpp>
#include <classical/io/read_bench.hpp>
#include <classical/io/read_symmetries.hpp>
#include <classical/io/read_unateness.hpp>
#include <classical/io/read_verilog.hpp>
#include <classical/io/write_aiger.hpp>
#include <classical/io/write_bench.hpp>
#include <classical/io/write_verilog.hpp>
#include <classical/mig/mig_to_aig.hpp>
#include <classical/mig/mig_from_string.hpp>
#include <classical/mig/mig_utils.hpp>
#include <classical/mig/mig_verilog.hpp>
#include <classical/xmg/xmg_aig.hpp>
#include <classical/xmg/xmg_cover.hpp>
#include <classical/xmg/xmg_expr.hpp>
#include <classical/xmg/xmg_io.hpp>
#include <classical/xmg/xmg_lut.hpp>
#include <classical/xmg/xmg_mig.hpp>
#include <classical/xmg/xmg_show.hpp>
#include <classical/xmg/xmg_string.hpp>
#include <classical/xmg/xmg_utils.hpp>

namespace alice
{

using namespace cirkit;

/******************************************************************************
 * aig_graph                                                                  *
 ******************************************************************************/

template<>
std::string store_entry_to_string<aig_graph>( const aig_graph& aig )
{
  const auto& info = aig_info( aig );
  const auto& name = info.model_name;
  return boost::str( boost::format( "%s i/o = %d/%d" ) % ( name.empty() ? "(unnamed)" : name ) % info.inputs.size() % info.outputs.size() );
}

show_store_entry<aig_graph>::show_store_entry( command& cmd )
{
  boost::program_options::options_description aig_options( "AIG options" );

  aig_options.add_options()
    ( "levels", boost::program_options::value<unsigned>()->default_value( 0u ), "Compute and annotate levels for dot\n0: don't compute\n1: push to inputs\n2: push to outputs" )
    ;

  cmd.opts.add( aig_options );
}

bool show_store_entry<aig_graph>::operator()( aig_graph& aig, const std::string& dotname, const command& cmd )
{
  auto settings = std::make_shared<properties>();
  settings->set( "verbose", cmd.is_set( "verbose" ) );
  const auto levels = cmd.vm["levels"].as<unsigned>();
  if ( levels > 0u )
  {
    auto cl_settings = std::make_shared<properties>();
    cl_settings->set( "verbose", cmd.is_set( "verbose" ) );
    cl_settings->set( "push_to_outputs", levels == 2u );
    auto annotation = get( boost::vertex_annotation, aig );

    const auto vertex_levels = compute_levels( aig, cl_settings );
    for ( const auto& p : vertex_levels )
    {
      annotation[p.first]["level"] = std::to_string( p.second );
    }

    settings->set( "vertex_levels", boost::optional<std::map<aig_node, unsigned>>( vertex_levels ) );
  }

  write_dot( aig, dotname, settings );

  return true;
}

command::log_opt_t show_store_entry<aig_graph>::log() const
{
  return boost::none;
}

template<>
void print_store_entry_statistics<aig_graph>( std::ostream& os, const aig_graph& aig )
{
  aig_print_stats( aig );
}

template<>
command::log_opt_t log_store_entry_statistics<aig_graph>( const aig_graph& aig )
{
  const auto& info = aig_info( aig );

  std::vector<aig_node> outputs;
  for ( const auto& output : info.outputs )
  {
    outputs += output.first.node;
  }

  std::vector<unsigned> depths;
  const auto depth = compute_depth( aig, outputs, depths );

  return command::log_opt_t({
      {"inputs", static_cast<int>( info.inputs.size() )},
      {"outputs", static_cast<int>( info.outputs.size() )},
      {"size", static_cast<int>( boost::num_vertices( aig ) - info.inputs.size() - 1u )},
      {"depth", static_cast<int>( depth )}});
}

template<>
aig_graph store_convert<tt, aig_graph>( const tt& t )
{
  return aig_from_truth_table( t );
}

template<>
bdd_function_t store_convert<aig_graph, bdd_function_t>( const aig_graph& aig )
{
  Cudd mgr;
  bdd_simulator simulator( mgr );
  auto values = simulate_aig( aig, simulator );

  std::vector<BDD> bdds;

  for ( const auto& o : aig_info( aig ).outputs )
  {
    bdds.push_back( values[o.first] );
  }

  return {mgr, bdds};
}

template<>
bool store_can_read_io_type<aig_graph, io_aiger_tag_t>( command& cmd )
{
  cmd.opts.add_options()
    ( "nosym",    "do not read symmetry file if existing" )
    ( "nounate",  "do not read unateness file if existing" )
    ( "nostrash", "do not strash the AIG when reading (in binary AIGER format)" )
    ;
  return true;
}

template<>
aig_graph store_read_io_type<aig_graph, io_aiger_tag_t>( const std::string& filename, const command& cmd )
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
  if ( !cmd.is_set( "nosym" ) && boost::filesystem::exists( symname ) )
  {
    /* read symmetries */
    std::cout << "[i] found and read symmetries file" << std::endl;
    read_symmetries( aig, symname );
  }

  /* auto-find unateness file */
  const auto depname = filename.substr( 0, filename.size() - 3 ) + "dep";
  if ( !cmd.is_set( "nounate" ) && boost::filesystem::exists( depname ) )
  {
    /* read unateness */
    std::cout << "[i] found and read unateness dependency file" << std::endl;
    read_unateness( aig, depname );
  }

  return aig;
}

template<>
aig_graph store_read_io_type<aig_graph, io_bench_tag_t>( const std::string& filename, const command& cmd )
{
  aig_graph aig;
  read_bench( aig, filename );
  return aig;
}

template<>
void store_write_io_type<aig_graph, io_aiger_tag_t>( const aig_graph& aig, const std::string& filename, const command& cmd )
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
aig_graph store_read_io_type<aig_graph, io_verilog_tag_t>( const std::string& filename, const command& cmd )
{
  return read_verilog_with_abc( filename );
}

template<>
void store_write_io_type<aig_graph, io_verilog_tag_t>( const aig_graph& aig, const std::string& filename, const command& cmd )
{
  write_verilog( aig, filename );
}

template<>
void store_write_io_type<aig_graph, io_edgelist_tag_t>( const aig_graph& aig, const std::string& filename, const command& cmd )
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
aig_graph store_convert<mig_graph, aig_graph>( const mig_graph& mig )
{
  return mig_to_aig( mig );
}

template<>
mig_graph store_convert<aig_graph, mig_graph>( const aig_graph& aig )
{
  return aig_to_mig( aig );
}

template<>
std::string store_entry_to_string<mig_graph>( const mig_graph& mig )
{
  const auto& info = mig_info( mig );
  const auto& name = info.model_name;
  return boost::str( boost::format( "%s i/o = %d/%d" ) % ( name.empty() ? "(unnamed)" : name ) % info.inputs.size() % info.outputs.size() );
}

show_store_entry<mig_graph>::show_store_entry( command& cmd )
{
}

bool show_store_entry<mig_graph>::operator()( mig_graph& mig, const std::string& dotname, const command& cmd )
{
  write_dot( mig, dotname );

  return true;
}

command::log_opt_t show_store_entry<mig_graph>::log() const
{
  return boost::none;
}

template<>
void print_store_entry_statistics<mig_graph>( std::ostream& os, const mig_graph& mig )
{
  mig_print_stats( mig, os );
}

template<>
command::log_opt_t log_store_entry_statistics<mig_graph>( const mig_graph& mig )
{
  const auto& info = mig_info( mig );

  std::vector<mig_node> outputs;
  for ( const auto& output : info.outputs )
  {
    outputs += output.first.node;
  }

  std::vector<unsigned> depths;
  const auto depth = compute_depth( mig, outputs, depths );

  return command::log_opt_t({
      {"inputs", static_cast<int>( info.inputs.size() )},
      {"outputs", static_cast<int>( info.outputs.size() )},
      {"size", static_cast<int>( boost::num_vertices( mig ) - info.inputs.size() - 1u )},
      {"depth", depth},
      {"complemented_edges", number_of_complemented_edges( mig )},
      {"inverters", number_of_inverters( mig )}
    });
}

template<>
expression_t::ptr store_convert<mig_graph, expression_t::ptr>( const mig_graph& mig )
{
  return mig_to_expression( mig, mig_info( mig ).outputs.front().first );
}

template<>
mig_graph store_convert<expression_t::ptr, mig_graph>( const expression_t::ptr& expr )
{
  mig_graph mig;
  mig_initialize( mig );
  std::vector<mig_function> pis;
  mig_create_po( mig, mig_from_expression( mig, pis, expr ), "f" );
  return mig;
}

template<>
void store_write_io_type<mig_graph, io_verilog_tag_t>( const mig_graph& mig, const std::string& filename, const command& cmd )
{
  write_verilog( mig, filename );
}

template<>
mig_graph store_read_io_type<mig_graph, io_verilog_tag_t>( const std::string& filename, const command& cmd )
{
  return read_mighty_verilog( filename );
}

/******************************************************************************
 * counterexample_t                                                           *
 ******************************************************************************/

template<>
std::string store_entry_to_string<counterexample_t>( const counterexample_t& cex )
{
  std::stringstream os;
  os << cex;
  return os.str();
}

/******************************************************************************
 * simple_fanout_graph_t                                                      *
 ******************************************************************************/

template<>
std::string store_entry_to_string<simple_fanout_graph_t>( const simple_fanout_graph_t& nl )
{
  return "";
}

/******************************************************************************
 * std::vector<aig_node>                                                      *
 ******************************************************************************/

template<>
std::string store_entry_to_string<std::vector<aig_node>>( const std::vector<aig_node>& g )
{
  return ( boost::format( "{ %s }" ) % any_join( g, ", " ) ).str();
}

template<>
void print_store_entry<std::vector<aig_node>>( std::ostream& os, const std::vector<aig_node>& g )
{
  os << boost::format( "{ %s }" ) % any_join( g, ", " ) << std::endl;
}

/******************************************************************************
 * tt                                                                         *
 ******************************************************************************/

template<>
std::string store_entry_to_string<tt>( const tt& t )
{
  std::stringstream os;
  os << t;
  return os.str();
}

template<>
void print_store_entry<tt>( std::ostream& os, const tt& t )
{
  os << tt_to_hex( t ) << std::endl
     << t << std::endl;
}

template<>
void store_write_io_type<tt, io_pla_tag_t>( const tt& t, const std::string& filename, const command& cmd )
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
 * expression_t::ptr                                                          *
 ******************************************************************************/

template<>
std::string store_entry_to_string<expression_t::ptr>( const expression_t::ptr& expr )
{
  std::stringstream s;
  s << expr;
  return s.str();
}

template<>
void print_store_entry_statistics<expression_t::ptr>( std::ostream& os, const expression_t::ptr& expr )
{
  os << std::endl;
}

template<>
command::log_opt_t log_store_entry_statistics<expression_t::ptr>( const expression_t::ptr& expr )
{
  return command::log_opt_t({
      {"expression", expression_to_string( expr )}
    });
}


template<>
void print_store_entry<expression_t::ptr>( std::ostream& os, const expression_t::ptr& expr )
{
  os << expr << std::endl;
}

template<>
tt store_convert<expression_t::ptr, tt>( const expression_t::ptr& expr )
{
  return tt_from_expression( expr );
}

template<>
bdd_function_t store_convert<expression_t::ptr, bdd_function_t>( const expression_t::ptr& expr )
{
  Cudd manager;
  return bdd_from_expression( manager, expr );
}

/******************************************************************************
 * xmg_graph                                                                  *
 ******************************************************************************/

template<>
std::string store_entry_to_string<xmg_graph>( const xmg_graph& xmg )
{
  const auto name = xmg.name();
  return boost::str( boost::format( "%s i/o = %d/%d" ) % ( name.empty() ? "(unnamed)" : name ) % xmg.inputs().size() % xmg.outputs().size() );
}

template<>
void print_store_entry_statistics<xmg_graph>( std::ostream& os, const xmg_graph& xmg )
{
  xmg_print_stats( xmg, os );
}

template<>
command::log_opt_t log_store_entry_statistics<xmg_graph>( const xmg_graph& xmg )
{
  auto log = command::log_map_t({
      {"inputs", static_cast<unsigned>( xmg.inputs().size() )},
      {"outputs", static_cast<unsigned>( xmg.outputs().size() )},
      {"size", xmg.num_gates()},
      {"maj", xmg.num_maj()},
      {"real_maj", compute_pure_maj_count( xmg )},
      {"xor", xmg.num_xor()},
      {"depth", compute_depth( xmg )},
    });

  if ( false )
  {
    xmg_graph xmg_copy = xmg;
    xmg_copy.compute_fanout();
    xmg_copy.compute_levels();

    std::vector<unsigned> fanouts( xmg.size() );
    ranges::transform( xmg_copy.nodes(), fanouts.begin(), [&xmg_copy]( xmg_node n ) { return xmg_copy.fanout_count( n ); } );

    std::vector<unsigned> level_diffs( ranges::size( xmg_copy.edges() ) );
    ranges::transform( xmg_copy.edges(), level_diffs.begin(), [&xmg_copy]( const xmg_edge& e ) {
        return xmg_copy.level( boost::source( e, xmg_copy.graph() ) ) - xmg_copy.level( boost::target( e, xmg_copy.graph() ) );
      } );

    log["fanouts"] = fanouts;
    log["level_diffs"] = level_diffs;
  }

  return log;
}

show_store_entry<xmg_graph>::show_store_entry( command& cmd )
{
  boost::program_options::options_description xmg_options( "XMG options" );

  xmg_options.add_options()
    ( "cover",                                                                          "dump LUT cover of XMG" )
    ( "show_all_edges",                                                                 "also show edges of AND and OR gates" )
    ( "show_node_ids",                                                                  "show node ids" )
    ( "format",         boost::program_options::value<unsigned>()->default_value( 0u ), "output format\n0: dot, 1: cytoscape (JS)\n" )
    ;

  cmd.opts.add( xmg_options );
}

bool show_store_entry<xmg_graph>::operator()( xmg_graph& xmg, const std::string& dotname, const command& cmd )
{
  switch ( cmd.vm["format"].as<unsigned>() )
  {
  case 0u:
    if ( cmd.is_set( "cover" ) )
    {
      if ( !xmg.has_cover() )
      {
        std::cout << "[w] XMG has no cover" << std::endl;
        return false;
      }

      xmg_cover_write_dot( xmg, dotname );
    }
    else
    {
      auto settings = std::make_shared<properties>();
      settings->set( "show_and_or_edges", cmd.is_set( "show_all_edges" ) );
      settings->set( "show_node_ids", cmd.is_set( "show_node_ids" ) );
      write_dot( xmg, dotname, settings );
    }
    return true;

  case 1u:
    {
      auto settings = std::make_shared<properties>();
      settings->set( "show_node_ids", cmd.is_set( "show_node_ids" ) );
      write_javascript_cytoscape( xmg, dotname, settings );
    }
    return true;

  default:
    std::cout << "[w] unknown format" << std::endl;
    return false;
  }
}

command::log_opt_t show_store_entry<xmg_graph>::log() const
{
  return boost::none;
}

template<>
expression_t::ptr store_convert<xmg_graph, expression_t::ptr>( const xmg_graph& xmg )
{
  return xmg_to_expression( xmg, xmg.outputs().front().first );
}

template<>
xmg_graph store_convert<expression_t::ptr, xmg_graph>( const expression_t::ptr& expr )
{
  xmg_graph xmg;
  std::vector<xmg_function> pis;
  xmg.create_po( xmg_from_expression( xmg, pis, expr ), "f" );
  return xmg;
}

template<>
xmg_graph store_convert<aig_graph, xmg_graph>( const aig_graph& aig )
{
  return xmg_from_aig( aig );
}

template<>
aig_graph store_convert<xmg_graph, aig_graph>( const xmg_graph& aig )
{
  return xmg_create_aig_topological( aig );
}

template<>
xmg_graph store_convert<mig_graph, xmg_graph>( const mig_graph& mig )
{
  return xmg_from_mig( mig );
}

template<>
mig_graph store_convert<xmg_graph, mig_graph>( const xmg_graph& mig )
{
  return xmg_create_mig_topological( mig );
}

template<>
void store_write_io_type<xmg_graph, io_bench_tag_t>( const xmg_graph& xmg, const std::string& filename, const command& cmd )
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
bool store_can_read_io_type<xmg_graph, io_verilog_tag_t>( command& cmd )
{
  boost::program_options::options_description xmg_options( "XMG options" );

  xmg_options.add_options()
    ( "as_mig", "read as MIG (translate XOR to MAJ)" )
    ( "no_strash", "disable structural hashing when reading the XMG" )
    ( "no_invprop", "disable inverter propagation when reading the XMG" )
    ;

  cmd.opts.add( xmg_options );

  return true;
}

template<>
xmg_graph store_read_io_type<xmg_graph, io_verilog_tag_t>( const std::string& filename, const command& cmd )
{
  return read_verilog( filename, !cmd.is_set( "as_mig" ), !cmd.is_set( "no_strash" ), !cmd.is_set( "no_invprop" ) );
}

template<>
bool store_can_write_io_type<xmg_graph, io_verilog_tag_t>( command& cmd )
{
  boost::program_options::options_description xmg_options( "XMG options" );

  xmg_options.add_options()
    ( "maj_module", "express MAJ gates as modules" )
    ;

  cmd.opts.add( xmg_options );

  return true;
}

template<>
void store_write_io_type<xmg_graph, io_verilog_tag_t>( const xmg_graph& xmg, const std::string& filename, const command& cmd )
{
  auto settings = std::make_shared<properties>();
  settings->set( "maj_module", cmd.is_set( "maj_module" ) );
  write_verilog( xmg, filename, settings );
}

template<>
xmg_graph store_read_io_type<xmg_graph, io_yig_tag_t>( const std::string& filename, const command& cmd )
{
  return xmg_read_yig( filename );
}

template<>
bool store_can_write_io_type<xmg_graph, io_smt_tag_t>( command& cmd )
{
  boost::program_options::options_description xmg_options( "XMG options" );

  xmg_options.add_options()
    ( "xor_blocks", "write XOR blocks" )
    ;

  cmd.opts.add( xmg_options );

  return true;

}

template<>
void store_write_io_type<xmg_graph, io_smt_tag_t>( const xmg_graph& xmg, const std::string& filename, const command& cmd )
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
