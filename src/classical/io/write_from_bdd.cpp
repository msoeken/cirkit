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
