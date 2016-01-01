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

#include "write_from_bdd.hpp"

#include <boost/algorithm/string/predicate.hpp>

#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>
#include <classical/aig.hpp>
#include <classical/dd/aig_from_cirkit_bdd.hpp>
#include <classical/io/write_aiger.hpp>
#include <classical/io/write_pla_from_cirkit_bdd.hpp>

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

void write_from_bdd( const std::vector<bdd>& fs, const std::string& filename,
                     const properties::ptr& settings,
                     const properties::ptr& statistics )
{
  /* settings */
  auto input_labels  = get( settings, "input_labels",  std::vector<std::string>() );
  auto output_labels = get( settings, "output_labels", std::vector<std::string>() );

  /* timing */
  properties_timer t( statistics );

  if ( boost::ends_with( filename, ".pla" ) )
  {
    std::ofstream out( filename.c_str(), std::ofstream::out );
    write_pla_from_cirkit_bdd( fs, input_labels, output_labels, out );
  }
  else if ( boost::ends_with( filename, ".aag" ) )
  {
    aig_graph aig;
    aig_initialize( aig );
    auto functions = aig_from_bdd( aig, fs, settings );
    assert ( functions.size() == fs.size() );
    for ( const auto& func : index( functions ) )
    {
      aig_create_po( aig, func.value, func.index < output_labels.size() ? output_labels.at( func.index ) : boost::str( boost::format( "f%d" ) % func.index ) );
    }
    write_aiger( aig, filename );
  }
  else
  {
    std::cerr << "[e] unknown suffix for file " << filename << std::endl;
    assert( false );
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
