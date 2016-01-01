/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
