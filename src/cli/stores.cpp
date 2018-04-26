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

#include <cstdio>
#include <functional>
#include <fstream>
#include <iostream>
#include <sstream>

#include <boost/graph/adjacency_list.hpp>

#include <core/graph/depth.hpp>
#include <core/utils/bitset_utils.hpp>
#include <core/utils/range_utils.hpp>

#include <classical/abc/utils/abc_run_command.hpp>
#include <classical/functions/aig_from_truth_table.hpp>
#include <classical/functions/aig_to_mig.hpp>
#include <classical/functions/compute_levels.hpp>
#include <classical/functions/simulate_aig.hpp>
#include <classical/mig/mig_to_aig.hpp>
#include <classical/mig/mig_from_string.hpp>
#include <classical/mig/mig_utils.hpp>
#include <classical/xmg/xmg_aig.hpp>
#include <classical/xmg/xmg_cover.hpp>
#include <classical/xmg/xmg_expr.hpp>
#include <classical/xmg/xmg_lut.hpp>
#include <classical/xmg/xmg_mig.hpp>
#include <classical/xmg/xmg_show.hpp>
#include <classical/xmg/xmg_string.hpp>
#include <classical/xmg/xmg_utils.hpp>

#include <fmt/format.h>

namespace alice
{

using namespace cirkit;

/******************************************************************************
 * bdd_function_t                                                             *
 ******************************************************************************/

template<>
std::string to_string<bdd_function_t>( const bdd_function_t& bdd )
{
  return fmt::format( "{} variables, {} functions, {} nodes",
                      bdd.first.ReadSize(),
                      bdd.second.size(),
                      bdd.first.ReadKeys() );
}

template<>
void print<bdd_function_t>( std::ostream& os, const bdd_function_t& bdd )
{
  for ( const auto& f : index( bdd.second ) )
  {
    os << "Function " << f.index << std::endl;
    f.value.PrintMinterm();
    os << std::endl;
  }
}

template<>
bool can_show<bdd_function_t>( std::string& extension, command& cmd )
{
  extension = "dot";

  cmd.add_flag( "--add", "convert BDD to ADD to have no compemented edges" )->group( "BDDs" );

  return true;
}

template<>
void show<bdd_function_t>( std::ostream& out, const bdd_function_t& bdd, const command& cmd )
{
  const std::string tmp = std::tmpnam( nullptr );
  auto * fd = fopen( tmp.c_str(), "w" );

  if ( cmd.is_set( "add" ) )
  {
    std::vector<ADD> adds( bdd.second.size() );
    std::transform( bdd.second.begin(), bdd.second.end(), adds.begin(), std::bind( &BDD::Add, std::placeholders::_1 ) );
    bdd.first.DumpDot( adds, 0, 0, fd );
  }
  else
  {
    bdd.first.DumpDot( bdd.second, 0, 0, fd );
  }
  fclose( fd );

  std::ifstream in( tmp.c_str(), std::ifstream::in );
  std::string line;
  while ( std::getline( in, line ) )
  {
    out << line;
  }
  in.close();

  std::remove( tmp.c_str() );
}

template<>
void print_statistics<bdd_function_t>( std::ostream& os, const bdd_function_t& bdd )
{
  std::vector<double> minterms;

  for ( const auto& f : bdd.second )
  {
    minterms.push_back( f.CountMinterm( bdd.first.ReadSize() ) );
  }

  os << "[i] no. of variables: " << bdd.first.ReadSize() << std::endl
     << "[i] no. of nodes:     " << bdd.first.ReadKeys() << std::endl
     << "[i] no. of minterms:  " << any_join( minterms, " " ) << std::endl
     << "[i] level sizes:      " << any_join( level_sizes( bdd.first, bdd.second ), " " ) << std::endl
     << "[i] maximum fanout:   " << maximum_fanout( bdd.first, bdd.second ) << std::endl
     << "[i] complement edges: " << count_complement_edges( bdd.first, bdd.second ) << std::endl;


  for ( const auto& p : index( bdd.second ) )
  {
    os << "[i] info for output " << p.index << ":" << std::endl
       << "[i] - path count:               " << p.value.CountPath() << std::endl
       << "[i] - path count (to non-zero): " << Cudd_CountPathsToNonZero( p.value.getNode() ) << std::endl;
  }

  bdd.first.info();
}

template<>
nlohmann::json log_statistics<bdd_function_t>( const bdd_function_t& bdd )
{
  return nlohmann::json({
      {"inputs", bdd.first.ReadSize()},
      {"outputs", static_cast<unsigned>( bdd.second.size() )}
    });
}

/******************************************************************************
 * aig_graph                                                                  *
 ******************************************************************************/

template<>
std::string to_string<aig_graph>( const aig_graph& aig )
{
  const auto& info = aig_info( aig );
  const auto& name = info.model_name;
  return fmt::format( "{} i/o = {}/{}",
                      name.empty() ? "(unnamed)" : name,
                      info.inputs.size(),
                      info.outputs.size() );
}

template<>
bool can_show<aig_graph>( std::string& extension, command& cmd )
{
  extension = "dot";

  cmd.add_option<unsigned>( "--levels", "compute and annotate levels for dot: 0 don't compute (default), 1: push to inputs, 2: push to outputs" )->group( "AIGs" );

  return true;
}

template<>
void show<aig_graph>( std::ostream& out, const aig_graph& aig, const command& cmd )
{
  aig_graph copy = aig;

  auto settings = std::make_shared<properties>();
  settings->set( "verbose", cmd.is_set( "verbose" ) );
  const auto levels = cmd.option_value<unsigned>( "--levels", 0u );
  if ( levels > 0u )
  {
    auto cl_settings = std::make_shared<properties>();
    cl_settings->set( "verbose", cmd.is_set( "verbose" ) );
    cl_settings->set( "push_to_outputs", levels == 2u );
    auto annotation = get( boost::vertex_annotation, copy );

    const auto vertex_levels = compute_levels( copy, cl_settings );
    for ( const auto& p : vertex_levels )
    {
      annotation[p.first]["level"] = std::to_string( p.second );
    }

    settings->set( "vertex_levels", boost::optional<std::map<aig_node, unsigned>>( vertex_levels ) );
  }

  write_dot( copy, out, settings );
}

template<>
void print_statistics<aig_graph>( std::ostream& os, const aig_graph& aig )
{
  aig_print_stats( aig );
}

template<>
nlohmann::json log_statistics<aig_graph>( const aig_graph& aig )
{
  const auto& info = aig_info( aig );

  std::vector<aig_node> outputs;
  for ( const auto& output : info.outputs )
  {
    outputs.push_back( output.first.node );
  }

  std::vector<unsigned> depths;
  const auto depth = compute_depth( aig, outputs, depths );

  return nlohmann::json({
      {"inputs", static_cast<int>( info.inputs.size() )},
      {"outputs", static_cast<int>( info.outputs.size() )},
      {"size", static_cast<int>( boost::num_vertices( aig ) - info.inputs.size() - 1u )},
      {"depth", static_cast<int>( depth )}});
}

template<>
aig_graph convert<tt, aig_graph>( const tt& t )
{
  return aig_from_truth_table( to_kitty( t ) );
}

template<>
bdd_function_t convert<aig_graph, bdd_function_t>( const aig_graph& aig )
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

/******************************************************************************
 * mig_graph                                                                  *
 ******************************************************************************/

template<>
aig_graph convert<mig_graph, aig_graph>( const mig_graph& mig )
{
  return mig_to_aig( mig );
}

template<>
mig_graph convert<aig_graph, mig_graph>( const aig_graph& aig )
{
  return aig_to_mig( aig );
}

template<>
std::string to_string<mig_graph>( const mig_graph& mig )
{
  const auto& info = mig_info( mig );
  const auto& name = info.model_name;
  return fmt::format( "{} i/o = {}/{}",
                      name.empty() ? "(unnamed)" : name,
                      info.inputs.size(),
                      info.outputs.size() );
}

template<>
bool can_show<mig_graph>( std::string& extension, command& cmd )
{
  extension = "dot";
  return true;
}

template<>
void show<mig_graph>( std::ostream& out, const mig_graph& mig, const command& cmd )
{
  write_dot( mig, out );
}

template<>
void print_statistics<mig_graph>( std::ostream& os, const mig_graph& mig )
{
  mig_print_stats( mig, os );
}

template<>
nlohmann::json log_statistics<mig_graph>( const mig_graph& mig )
{
  const auto& info = mig_info( mig );

  std::vector<mig_node> outputs;
  for ( const auto& output : info.outputs )
  {
    outputs.push_back( output.first.node );
  }

  std::vector<unsigned> depths;
  const auto depth = compute_depth( mig, outputs, depths );

  return nlohmann::json({
      {"inputs", static_cast<int>( info.inputs.size() )},
      {"outputs", static_cast<int>( info.outputs.size() )},
      {"size", static_cast<int>( boost::num_vertices( mig ) - info.inputs.size() - 1u )},
      {"depth", depth},
      {"complemented_edges", number_of_complemented_edges( mig )},
      {"inverters", number_of_inverters( mig )}
    });
}

template<>
expression_t::ptr convert<mig_graph, expression_t::ptr>( const mig_graph& mig )
{
  return mig_to_expression( mig, mig_info( mig ).outputs.front().first );
}

template<>
mig_graph convert<expression_t::ptr, mig_graph>( const expression_t::ptr& expr )
{
  mig_graph mig;
  mig_initialize( mig );
  std::vector<mig_function> pis;
  mig_create_po( mig, mig_from_expression( mig, pis, expr ), "f" );
  return mig;
}

/******************************************************************************
 * counterexample_t                                                           *
 ******************************************************************************/

template<>
std::string to_string<counterexample_t>( const counterexample_t& cex )
{
  std::stringstream os;
  os << cex;
  return os.str();
}

/******************************************************************************
 * simple_fanout_graph_t                                                      *
 ******************************************************************************/

template<>
std::string to_string<simple_fanout_graph_t>( const simple_fanout_graph_t& nl )
{
  return "";
}

/******************************************************************************
 * std::vector<aig_node>                                                      *
 ******************************************************************************/

template<>
std::string to_string<std::vector<aig_node>>( const std::vector<aig_node>& g )
{
  return fmt::format( "{{ {} }}", any_join( g, ", " ) );
}

template<>
void print<std::vector<aig_node>>( std::ostream& os, const std::vector<aig_node>& g )
{
  os << fmt::format( "{{ {} }}", any_join( g, ", " ) );
}

/******************************************************************************
 * tt                                                                         *
 ******************************************************************************/

template<>
std::string to_string<tt>( const tt& t )
{
  std::stringstream os;
  os << t;
  return os.str();
}

template<>
void print<tt>( std::ostream& os, const tt& t )
{
  os << tt_to_hex( t ) << std::endl
     << t << std::endl;
}

/******************************************************************************
 * expression_t::ptr                                                          *
 ******************************************************************************/

template<>
std::string to_string<expression_t::ptr>( const expression_t::ptr& expr )
{
  std::stringstream s;
  s << expr;
  return s.str();
}

template<>
void print_statistics<expression_t::ptr>( std::ostream& os, const expression_t::ptr& expr )
{
  os << std::endl;
}

template<>
nlohmann::json log_statistics<expression_t::ptr>( const expression_t::ptr& expr )
{
  return nlohmann::json({
      {"expression", expression_to_string( expr )}
    });
}


template<>
void print<expression_t::ptr>( std::ostream& os, const expression_t::ptr& expr )
{
  os << expr << std::endl;
}

template<>
tt convert<expression_t::ptr, tt>( const expression_t::ptr& expr )
{
  return tt_from_expression( expr );
}

template<>
bdd_function_t convert<expression_t::ptr, bdd_function_t>( const expression_t::ptr& expr )
{
  Cudd manager;
  return bdd_from_expression( manager, expr );
}

/******************************************************************************
 * xmg_graph                                                                  *
 ******************************************************************************/

template<>
std::string to_string<xmg_graph>( const xmg_graph& xmg )
{
  const auto name = xmg.name();
  return fmt::format( "{} i/o = {}/{}",
                      name.empty() ? "(unnamed)" : name,
                      xmg.inputs().size(),
                      xmg.outputs().size() );
}

template<>
void print_statistics<xmg_graph>( std::ostream& os, const xmg_graph& xmg )
{
  xmg_print_stats( xmg, os );
}

template<>
nlohmann::json log_statistics<xmg_graph>( const xmg_graph& xmg )
{
  auto log = nlohmann::json({
      {"inputs", static_cast<unsigned>( xmg.inputs().size() )},
      {"outputs", static_cast<unsigned>( xmg.outputs().size() )},
      {"size", xmg.num_gates()},
      {"maj", xmg.num_maj()},
      {"real_maj", compute_pure_maj_count( xmg )},
      {"xor", xmg.num_xor()},
      {"depth", compute_depth( xmg )},
    });

  return log;
}

template<>
bool can_show<xmg_graph>( std::string& extension, command& cmd )
{
  extension = "dot";

  cmd.add_flag( "--cover", "dump LUT cover of XMG" )->group( "XMGs" );
  cmd.add_flag( "--show_all_edges", "also show edges of AND and OR gates" )->group( "XMGs" );
  cmd.add_flag( "--show_node_ids", "show node ids" )->group( "XMGs" );
  cmd.add_option<unsigned>( "--format", "output format: 0: dot (default), 1: cytoscape (JS)" )->group( "XMGs" );

  return true;
}

template<>
void show<xmg_graph>( std::ostream& out, const xmg_graph& xmg, const command& cmd )
{
  switch ( cmd.option_value<unsigned>( "--format", 0u ) )
  {
  case 0u:
    if ( cmd.is_set( "cover" ) )
    {
      if ( !xmg.has_cover() )
      {
        cmd.env->out() << "[w] XMG has no cover" << std::endl;
        return;
      }

      xmg_cover_write_dot( xmg, out );
    }
    else
    {
      auto settings = std::make_shared<properties>();
      settings->set( "show_and_or_edges", cmd.is_set( "show_all_edges" ) );
      settings->set( "show_node_ids", cmd.is_set( "show_node_ids" ) );

      xmg_graph copy = xmg;
      write_dot( copy, out, settings );
    }
    break;

  case 1u:
    {
      auto settings = std::make_shared<properties>();
      settings->set( "show_node_ids", cmd.is_set( "show_node_ids" ) );

      xmg_graph copy = xmg;
      write_javascript_cytoscape( copy, out, settings );
    }
    break;

  default:
    cmd.env->out() << "[w] unknown format" << std::endl;
  }
}

template<>
expression_t::ptr convert<xmg_graph, expression_t::ptr>( const xmg_graph& xmg )
{
  return xmg_to_expression( xmg, xmg.outputs().front().first );
}

template<>
xmg_graph convert<expression_t::ptr, xmg_graph>( const expression_t::ptr& expr )
{
  xmg_graph xmg;
  std::vector<xmg_function> pis;
  xmg.create_po( xmg_from_expression( xmg, pis, expr ), "f" );
  return xmg;
}

template<>
xmg_graph convert<aig_graph, xmg_graph>( const aig_graph& aig )
{
  return xmg_from_aig( aig );
}

template<>
aig_graph convert<xmg_graph, aig_graph>( const xmg_graph& aig )
{
  return xmg_create_aig_topological( aig );
}

template<>
xmg_graph convert<mig_graph, xmg_graph>( const mig_graph& mig )
{
  return xmg_from_mig( mig );
}

template<>
mig_graph convert<xmg_graph, mig_graph>( const xmg_graph& mig )
{
  return xmg_create_mig_topological( mig );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
