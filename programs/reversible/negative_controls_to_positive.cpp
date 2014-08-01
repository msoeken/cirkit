/* RevKit (www.rekit.org)
 * Copyright (C) 2009-2014  University of Bremen
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

#include <reversible/circuit.hpp>
#include <reversible/functions/negative_controls_to_positive.hpp>
#include <reversible/io/read_realization.hpp>
#include <reversible/io/write_realization.hpp>
#include <reversible/utils/reversible_program_options.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  reversible_program_options opts;
  opts.add_read_realization_option();
  opts.add_write_realization_option();
  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_write_realization_filename_set() )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  circuit src, dest;
  read_realization( src, opts.read_realization_filename() );
  negative_controls_to_positive( src, dest );
  write_realization( dest, opts.write_realization_filename() );

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
