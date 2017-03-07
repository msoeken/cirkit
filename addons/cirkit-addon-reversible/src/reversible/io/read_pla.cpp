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

  inline constant transform_pla_to_constants( const char& c )
  {
    switch ( c ) {
    case '-':
    case '~':
      return constant();

    case '0':
    case '1':
      return constant( c == '1' );

    default:
      assert( false );
      return constant();
    }
  }

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
      auto it = cube_map.find( in );
      if ( it == cube_map.end() )
      {
        cube_map.insert( {in, out} );
      }
      else
      {
        std::string new_out;
        for ( unsigned i = 0u; i < out.size(); ++i )
        {
          auto v1 = out[i]; auto v2 = it->second[i];
          new_out += ( v1 == '1' || v2 == '1' ) ? '1' : '0';
        }
        it->second = out;
      }
    }

    void on_end()
    {
      for ( const auto& p : cube_map )
      {
        std::vector<boost::optional<bool> > cube_in( p.first.size() );
        boost::transform( p.first, cube_in.begin(), transform_pla_to_constants );

        std::vector<boost::optional<bool> > cube_out( p.second.size() );
        boost::transform( p.second, cube_out.begin(), transform_pla_to_constants );

        spec.add_entry( cube_in, cube_out );
      }
    }

  private:
    binary_truth_table& spec;
    std::map<std::string, std::string> cube_map;
  };

  class read_pla_size_processor : public pla_processor
  {
  public:
    void on_num_inputs( unsigned num_inputs )
    {
      n = num_inputs;
    }

    void on_num_outputs( unsigned num_outputs )
    {
      m = num_outputs;
    }

    unsigned n, m;
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

  std::pair<unsigned, unsigned> read_pla_size( const std::string& filename )
  {
    read_pla_size_processor p;
    pla_parser( filename, p, true );
    return std::make_pair( p.n, p.m );
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
