/* RevKit (www.revkit.org)
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

#include "extract_pla.hpp"

#include <fstream>

#include <core/io/pla_parser.hpp>
#include <core/utils/range_utils.hpp>

namespace cirkit
{

class extract_pla_processor : public pla_processor
{
public:
  extract_pla_processor( std::ostream& os, unsigned output )
    : os( os ),
      output( output ) {}

  void on_num_inputs( unsigned num_inputs )
  {
    os << ".i " << num_inputs << std::endl
       << ".o 1" << std::endl;
  }

  void on_input_labels( const std::vector<std::string>& input_labels )
  {
    os << ".ilb " << any_join( input_labels, " " ) << std::endl;
  }

  void on_output_labels( const std::vector<std::string>& output_labels )
  {
    os << ".ob " << output_labels[output] << std::endl;
  }

  void on_end()
  {
    os << ".e" << std::endl;
  }

  void on_cube( const std::string& in, const std::string& out )
  {
    if ( out[output] == '1' )
    {
      os << in << " 1" << std::endl;
    }
  }

  std::ostream& os;
  unsigned output;
};

void extract_pla( const std::string& filename, const std::string& destination, unsigned output )
{
  std::ofstream os( destination.c_str(), std::ofstream::out );
  extract_pla_processor processor( os, output );
  pla_parser( filename, processor );
  os.close();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
