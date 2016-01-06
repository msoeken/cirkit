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

/**
 * @author Mathias Soeken
 */

#include <functional>

#include <boost/assign/std/vector.hpp>

#include <reversible/circuit.hpp>
#include <reversible/functions/negative_controls_to_positive.hpp>
#include <reversible/functions/fredkin_gates_to_toffoli.hpp>
#include <reversible/io/read_realization.hpp>
#include <reversible/io/write_realization.hpp>
#include <reversible/utils/reversible_program_options.hpp>

using namespace boost::assign;
using namespace cirkit;

int main( int argc, char ** argv )
{
  reversible_program_options opts;
  opts.add_read_realization_option();
  opts.add_write_realization_option();
  opts.add_options()
    ( "negative_to_positive,n", "Transform negative control lines to positive control lines" )
    ( "fredkin_to_toffoli,f",   "Transform Fredkin gates to Toffoli gates" )
    ;
  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_write_realization_filename_set() )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  std::vector<circuit> circuits( 1u );
  read_realization( circuits.front(), opts.read_realization_filename() );

  using func_t = std::function<void(const circuit&, circuit&)>;
  std::vector<std::pair<std::string, func_t>> transformations = {
    { "negative_to_positive", func_t( negative_controls_to_positive ) },
    { "fredkin_to_toffoli",   func_t( fredkin_gates_to_toffoli ) }
  };

  for ( const auto& t : transformations )
  {
    if ( opts.is_set( t.first ) )
    {
      circuits.resize( circuits.size() + 1u );
      t.second( circuits[circuits.size() - 2u], circuits[circuits.size() - 1u] );
    }
  }

  write_realization( circuits.back(), opts.write_realization_filename() );

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
