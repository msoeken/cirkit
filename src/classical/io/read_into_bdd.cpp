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

#include "read_into_bdd.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/range/algorithm.hpp>

#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>
#include <classical/aig.hpp>
#include <classical/dd/aig_to_cirkit_bdd.hpp>
#include <classical/io/read_aiger.hpp>
#include <classical/io/read_pla_to_cirkit_bdd.hpp>
#include <classical/utils/aig_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

std::pair<bdd_manager_ptr, std::vector<bdd>> read_into_bdd( const std::string& filename,
                                                            const properties::ptr& settings,
                                                            const properties::ptr& statistics )
{
  /* settings */
  auto log_max_objs = get( settings, "log_max_objs", 24u );

  /* timing */
  properties_timer t( statistics );

  if ( boost::ends_with( filename, ".pla" ) )
  {
    auto function = read_pla_into_cirkit_bdd( filename, settings );
    std::vector<bdd> fs( function->num_outputs() );
    for ( auto i = 0u; i < function->num_outputs(); ++i )
    {
      fs[i] = function->lookupOutput( i );
    }

    set( statistics, "input_labels", function->input_labels() );
    set( statistics, "output_labels", function->output_labels() );

    return std::make_pair( function->manager(), fs );
  }
  else if ( boost::ends_with( filename, ".aag" ) )
  {
    aig_graph aig;
    read_aiger( aig, filename );

    std::vector<bdd> fs;
    cirkit_bdd_simulator sim( aig, log_max_objs );
    auto map = simulate_aig( aig, sim );

    for ( const auto& m : map )
    {
      fs += m.second;
    }

    if ( statistics )
    {
      const auto info = aig_info( aig );
      std::vector<std::string> input_labels( info.inputs.size() );
      boost::transform( info.inputs, input_labels.begin(), [&]( const aig_node& n ) { return info.node_names.at( n ); } );
      auto output_labels = get_map_values( info.outputs );
      set( statistics, "input_labels", input_labels );
      set( statistics, "output_labels", output_labels );
    }

    return std::make_pair( sim.mgr, fs );
  }

  std::cerr << "[e] unknown suffix" << std::endl;
  assert( false );

  return std::make_pair( bdd_manager_ptr(), std::vector<bdd>() );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
