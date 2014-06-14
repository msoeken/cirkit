/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include "read_pla.hpp"

#include <fstream>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/range/algorithm.hpp>

#include <core/io/pla_parser.hpp>
#include <reversible/functions/extend_truth_table.hpp>

namespace cirkit
{

  struct transform_pla_to_constants
  {
    constant operator()( const char& c ) const
    {
      switch ( c ) {
      case '-':
      case '~':
        return constant();
        break;

      case '0':
      case '1':
        return constant( c == '1' );
        break;

      default:
        assert( false );
        return constant();
        break;
      }
    }
  };

  // A function to combine an input and an output cube
  //   @assumes that c1 and c2 have the same size
  binary_truth_table::cube_type combine_pla_cube( const binary_truth_table::cube_type& c1, const binary_truth_table::cube_type& c2 )
  {
    using namespace boost::assign;

    binary_truth_table::cube_type r;

    for ( unsigned i = 0u; i < c1.size(); ++i )
    {
      const binary_truth_table::value_type& v1 = c1.at( i );
      const binary_truth_table::value_type& v2 = c2.at( i );
      r += (v1 && *v1) || (v2 && *v2);
    }

    return r;
  };

  class read_pla_processor : public pla_processor
  {
  public:
    read_pla_processor( binary_truth_table& _spec ) : spec( _spec )
    {
      spec.clear();
    }

    void on_input_labels( const std::vector<std::string>& input_labels )
    {
      spec.set_inputs( input_labels );
    }

    void on_output_labels( const std::vector<std::string>& output_labels )
    {
      spec.set_outputs( output_labels );
    }

    void on_cube( const std::string& in, const std::string& out )
    {
      std::vector<boost::optional<bool> > cube_in( in.size() );
      boost::transform( in, cube_in.begin(), transform_pla_to_constants() );

      std::vector<boost::optional<bool> > cube_out( out.size() );
      boost::transform( out, cube_out.begin(), transform_pla_to_constants() );

      bool match = false;
      for ( binary_truth_table::iterator spec_cube = spec.begin(); spec_cube != spec.end(); ++spec_cube )
      {
        binary_truth_table::cube_type spec_in( spec_cube->first.first, spec_cube->first.second );
        binary_truth_table::cube_type spec_out( spec_cube->second.first, spec_cube->second.second );

        if ( cube_in == spec_in )
        {
          spec.remove_entry( spec_cube );
          spec.add_entry( cube_in, combine_pla_cube( cube_out, spec_out ) );
          match = true;
          break;
        }
      }

      if ( !match )
      {
        spec.add_entry( cube_in, cube_out );
      }
    }

  private:
    binary_truth_table& spec;
  };

  bool read_pla( binary_truth_table& spec, std::istream& in, const read_pla_settings& settings, std::string* error )
  {
    read_pla_processor p( spec );
    pla_parser( in, p, settings.skip_after_first_cube );

    if ( settings.extend )
    {
      extend_truth_table( spec );
    }

    return true;
  }

  bool read_pla( binary_truth_table& spec, const std::string& filename, const read_pla_settings& settings, std::string* error )
  {
    std::ifstream is;
    is.open( filename.c_str(), std::ifstream::in );

    if ( !is.good() )
    {
      if ( error )
      {
        *error = "Cannot open " + filename;
      }
      return false;
    }

    return read_pla( spec, is, settings, error );
  }

}

// Local Variables:
// c-basic-offset: 2
// End:
