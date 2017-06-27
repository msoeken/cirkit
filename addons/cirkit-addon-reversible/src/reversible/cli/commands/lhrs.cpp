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

#include <boost/program_options.hpp>

#include <core/utils/program_options.hpp>
#include <core/utils/temporary_filename.hpp>
#include <classical/abc/gia/gia_utils.hpp>
#include <classical/abc/utils/abc_run_command.hpp>
#include <classical/io/read_blif.hpp>
#include <classical/optimization/exorcism_minimization.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/synthesis/lut_based_synthesis.hpp>

using boost::program_options::value;

namespace cirkit
{

lhrs_command::lhrs_command( const environment::ptr& env )
  : aig_base_command( env, "LUT-based hierarchical reversible synthesis" )
{
  opts.add_options()
    ( "cut_size,k",         value_with_default( &cut_size ),                  "cut size" )
    ( "lutdecomp,l",                                                          "apply LUT decomposition technique where possible" )
    ( "additional_ancilla", value_with_default( &params.additional_ancilla ), "number of additional ancilla to add to circuit" )
    ( "progress,p",                                                           "show progress" )
    ( "onlylines",                                                            "do not create gates (useful for qubit estimation)" )
    ( "area_iters_init",    value_with_default( &area_iters_init ),           "number of exact area recovery iterations (in initial mapping)" )
    ( "flow_iters_init",    value_with_default( &flow_iters_init ),           "number of area flow recovery iterations (in initial mapping)" )
    ;

  boost::program_options::options_description esopdecomp_options( "ESOP decomposition options" );
  esopdecomp_options.add_options()
    ( "esopscript",      value_with_default( &esopscript ),      "ESOP optimization script\ndef: default exorcism script\ndef_wo4: default without exorlink-4\nnone: do not optimize ESOP cover" )
    ( "esopcovermethod", value_with_default( &esopcovermethod ), "ESOP cover method\naig: directly from AIG\nbdd: using PSDKRO method from BDD\naignew: new AIG-based method\nauto: tries to estimate the best method for each LUT" )
    ( "esoppostopt",                                             "Post-optimize network derived from ESOP synthesis" )
    ;
  opts.add( esopdecomp_options );

  boost::program_options::options_description lutdecomp_options( "LUT decomposition options" );
  lutdecomp_options.add_options()
    ( "satlut,s",                                                 "optimize mapping with SAT where possible" )
    ( "area_iters",   value_with_default( &params.area_iters ),   "number of exact area recovery iterations" )
    ( "flow_iters",   value_with_default( &params.flow_iters ),   "number of area flow recovery iterations" )
    ( "class_method", value_with_default( &params.class_method ), "classification method\n0: spectral classification\n1: affine classificiation" )
    ;
  opts.add( lutdecomp_options );

  boost::program_options::options_description debug_options( "Debug options" );
  debug_options.add_options()
    ( "dumpfile",       value( &params.dumpfile ), "name of existing directory to dump AIG and ESOP files for exorcism minimization" )
    ( "bounds",                                    "compute lower and upper bounds for qubits" )
    ( "dotname_mapped", value( &dotname_mapped ),  "filename to dump DOT representation of initial mapped network" )
    ;
  opts.add( debug_options );

  be_verbose();
  add_new_option();
}

command::rules_t lhrs_command::validity_rules() const
{
  return {
    {has_store_element<aig_graph>( env )},
    {[this]() { return esopscript == "def" || esopscript == "def_wo4" || esopscript == "none"; }, "unknown exorcism script"},
    {[this]() { return esopcovermethod == "aig" || esopcovermethod == "bdd" || esopcovermethod == "aignew" || esopcovermethod == "auto"; }, "unknown cover extraction method"}
  };
}

bool lhrs_command::execute()
{
  auto& circuits = env->store<circuit>();
  extend_if_new( circuits );

  const auto settings = make_settings();
  params.verbose = is_verbose();
  params.lutdecomp = is_set( "lutdecomp" );
  params.progress = is_set( "progress" );
  params.onlylines = is_set( "onlylines" );
  if ( esopcovermethod == "aig" )
  {
    params.cover_method = gia_graph::esop_cover_method::aig;
  }
  else if ( esopcovermethod == "bdd" )
  {
    params.cover_method = gia_graph::esop_cover_method::bdd;
  }
  else if ( esopcovermethod == "aignew" )
  {
    params.cover_method = gia_graph::esop_cover_method::aig_new;
  }
  else
  {
    params.cover_method = gia_graph::esop_cover_method::aig_threshold;
  }
  params.optimize_esop = esopscript != "none";
  params.optimize_postesop = is_set( "esoppostopt" );
  if ( esopscript != "none" )
  {
    params.script = script_from_string( esopscript );
  }
  params.satlut = is_set( "satlut" );

  circuit circ;

  const auto gia = gia_graph( aig() );
  const auto lut = gia.if_mapping( make_settings_from( std::make_pair( "lut_size", cut_size ), "area_mapping", std::make_pair( "area_iters", area_iters_init ), std::make_pair( "flow_iters", flow_iters_init ) ) );
  lut_based_synthesis( circuits.current(), lut, params, stats );
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

  print_runtime( stats.runtime );

  return true;
}

command::log_opt_t lhrs_command::log() const
{
  log_map_t map({
      {"runtime", stats.runtime},
      {"cut_size", cut_size},
      {"lut_count", lut_count},
      {"onlylines", is_set( "onlylines" )},
      {"area_iters_init", area_iters_init},
      {"flow_iters_init", flow_iters_init},
      {"esopscript", esopscript},
      {"esoppostopt", is_set( "esoppostopt" )},
      {"satlut", is_set( "satlut" )},
      {"area_iters", params.area_iters},
      {"flow_iters", params.flow_iters},
      {"class_method", params.class_method},
      {"num_decomp_default", stats.num_decomp_default},
      {"num_decomp_lut", stats.num_decomp_lut},
      {"exorcism_runtime", stats.exorcism_runtime},
      {"cover_runtime", stats.cover_runtime},
      {"class_counter", stats.class_counter},
      {"class_runtime", stats.class_runtime},
      {"mapping_runtime", stats.mapping_runtime}
    });

  if ( is_set( "bounds" ) )
  {
    map["debug_lb"] = debug_lb;
    map["debug_ub"] = lut_count;
  }

  return map;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
