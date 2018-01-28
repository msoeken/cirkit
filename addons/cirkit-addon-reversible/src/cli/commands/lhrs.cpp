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

#include "lhrs.hpp"

#include <sys/types.h>
#include <unistd.h>

#include <core/utils/temporary_filename.hpp>
#include <classical/abc/gia/gia_utils.hpp>
#include <classical/abc/utils/abc_run_command.hpp>
#include <classical/io/read_blif.hpp>
#include <classical/optimization/exorcism_minimization.hpp>
#include <classical/xmg/xmg_aig.hpp>
#include <classical/xmg/xmg_cover.hpp>
#include <cli/reversible_stores.hpp>
#include <reversible/synthesis/lhrs/legacy/lhrs.hpp>

namespace cirkit
{

lhrs_command::lhrs_command( const environment::ptr& env )
  : aig_base_command( env, "LUT-based hierarchical reversible synthesis" )
{
  /* General options */
  add_option( "--cut_size,-k", cut_size, "cut size", true );
  add_option( "--mapping_strategy,-m", mapping_strategy, "mapping strategy: direct (0): direct; min_db (1): LUT-based min DB; best_fit (2): LUT-based best fit; pick_best (3): LUT-based pick best", true );
  add_option( "--additional_ancilla", params.additional_ancilla, "number of additional ancilla to add to circuit", true );
  add_flag( "--progress,-p", params.progress, "show progress" );
  add_flag( "--onlylines", params.onlylines, "do not create gates (useful for qubit estimation)" );
  add_option( "--area_iters_init", area_iters_init, "number of exact area recovery iterations (in initial mapping)", true );
  add_option( "--flow_iters_init", flow_iters_init, "number of area flow recovery iterations (in initial mapping)", true );

  add_option( "--esopscript", script, "ESOP optimization script - def (1): default exorcism script, def_wo4 (2): default without exorlink-4, j2r (3): just two rounds, none (0): do not optimize ESOP cover", true )->group( "ESOP decomposition ");
  add_option( "--esopcovermethod", cover_method, "ESOP cover method - aig (0): directly from AIG, aignew (1): new AIG-based method, auto (2): tries to estimate the best method for each LUT, bdd (3): using PSDKRO method from BDD", true )->group( "ESOP decomposition ");
  add_flag( "--esoppostopt", params.map_esop_params.optimize_postesop, "post-optimize network derived from ESOP synthesis" )->group( "ESOP decomposition ");

  add_flag( "--satlut,-s", params.map_luts_params.satlut, "optimize mapping with SAT where possible" )->group( "LUT decomposition" );
  add_option( "--area_iters", params.map_luts_params.area_iters, "number of exact area recovery iterations", true )->group( "LUT decomposition" );
  add_option( "--flow_iters", params.map_luts_params.flow_iters, "number of area flow recovery iterations", true )->group( "LUT decomposition" );
  add_option( "--class_method", params.map_precomp_params.class_method, "classification method\n0: spectral classification\n1: affine classificiation", true )->group( "LUT decomposition" );

  add_option( "--dumpfile", params.map_esop_params.dumpfile, "name of existing directory to dump AIG and ESOP files for exorcism minimization" )->group( "Debug" );
  add_flag( "--nocollapse", params.map_esop_params.nocollapse, "do not collapse LUTs (useful with dumpfile to only write AIGs)" )->group( "Debug" );
  add_flag( "--count_costs", params.count_costs, "count costs and affected lines per single-target gate" )->group( "Debug" );
  add_flag( "--bounds", "compute lower and upper bounds for qubits" )->group( "Debug" );
  add_option( "--dotname_mapped", dotname_mapped, "filename to dump DOT representation of initial mapped network" )->group( "Debug" );

  be_verbose( params.verbose );
  add_new_option();
}

command::rules lhrs_command::validity_rules() const
{
  return {
    {has_store_element<aig_graph>( env )}
  };
}

void lhrs_command::execute()
{
  auto& circuits = env->store<circuit>();
  extend_if_new( circuits );

  std::istringstream ms_str( mapping_strategy );
  ms_str >> params.mapping_strategy;

  std::istringstream cm_str( cover_method );
  cm_str >> params.map_esop_params.cover_method;

  std::istringstream s_str( script );
  s_str >> params.map_esop_params.script;

  params.sync();

  circuit circ;

  stats = std::make_shared<legacy::lhrs_stats>();

  const auto gia = gia_graph( aig() );

  const auto lut = gia.if_mapping( make_settings_from( std::make_pair( "lut_size", cut_size ), "area_mapping", std::make_pair( "area_iters", area_iters_init ), std::make_pair( "flow_iters", flow_iters_init ), std::make_pair( "rounds", 7u ), std::make_pair( "rounds_ela", 7u ) ) );
  legacy::lut_based_synthesis( circuits.current(), /*xmg_from_gia( lut )*/ lut, params, *stats );

  // auto& xmg = env->store<xmg_graph>().current();
  // xmg_map( xmg, make_settings_from( std::make_pair( "cut_size", cut_size ) ) );
  // lut_based_synthesis( circuits.current(), xmg, params, *stats );

  lut_count = lut.lut_count();

  if ( is_set( "dotname_mapped" ) )
  {
    lut.write_dot_with_luts( dotname_mapped );
  }

  debug_lb = 0;
  if ( is_set( "bounds" ) )
  {
    lut.foreach_output( [&lut, this]( int index, int e ) {
        const auto driver = abc::Gia_ObjFaninId0p( lut, abc::Gia_ManCo( lut, e ) );
        debug_lb = std::max<unsigned>( debug_lb, abc::Gia_LutTFISize( lut, driver ) );
      } );
  }

  print_runtime( stats->runtime );
}

nlohmann::json lhrs_command::log() const
{
  nlohmann::json map({
      {"runtime", stats->runtime},
      {"cut_size", cut_size},
      {"lut_count", lut_count},
      {"onlylines", is_set( "onlylines" )},
      {"area_iters_init", area_iters_init},
      {"flow_iters_init", flow_iters_init},
      {"esoppostopt", is_set( "esoppostopt" )},
      {"satlut", is_set( "satlut" )},
      {"area_iters", params.map_luts_params.area_iters},
      {"flow_iters", params.map_luts_params.flow_iters},
      {"class_method", params.map_precomp_params.class_method},
      {"num_decomp_default", stats->num_decomp_default},
      {"num_decomp_lut", stats->num_decomp_lut},
      {"exorcism_runtime", stats->map_esop_stats.exorcism_runtime},
      {"cover_runtime", stats->map_esop_stats.cover_runtime},
      {"class_counter", stats->map_precomp_stats.class_counter},
      {"class_runtime", stats->map_precomp_stats.class_runtime},
      {"mapping_runtime", stats->map_luts_stats.mapping_runtime}
    });

  if ( is_set( "bounds" ) )
  {
    map["debug_lb"] = debug_lb;
    map["debug_ub"] = lut_count;
  }

  if ( is_set( "count_costs" ) )
  {
    map["gate_costs"] = stats->gate_costs;
    map["line_maps"] = stats->line_maps;
    map["affected_lines"] = stats->affected_lines;
    map["clean_ancillas"] = stats->clean_ancillas;
  }

  return map;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
