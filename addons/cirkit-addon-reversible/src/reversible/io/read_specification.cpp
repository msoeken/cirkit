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

#include "read_specification.hpp"

#include <fstream>
#include <iostream>

#include "revlib_parser.hpp"

namespace cirkit
{

  ////////////////////////////// class specification_processor
  class specification_processor::priv
  {
  public:
    priv( binary_truth_table& s )
    : spec( s ),
      has_entries( false ) {}

    binary_truth_table& spec;
    std::vector<constant> constants;
    std::vector<bool> garbage;
    bool has_entries;
  };

  specification_processor::specification_processor( binary_truth_table& spec )
    : revlib_processor(), d( new priv( spec ) )
  {
  }

  specification_processor::~specification_processor()
  {
    delete d;
  }

  void specification_processor::on_inputs( std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last ) const
  {
    std::vector<std::string> inputs( first, last );
    d->spec.set_inputs( inputs );
  }

  void specification_processor::on_outputs( std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last ) const
  {
    std::vector<std::string> outputs( first, last );
    d->spec.set_outputs( outputs );
  }

  void specification_processor::on_constants( std::vector<constant>::const_iterator first, std::vector<constant>::const_iterator last ) const
  {
    d->constants.assign( first, last );
  }

  void specification_processor::on_garbage( std::vector<bool>::const_iterator first, std::vector<bool>::const_iterator last ) const
  {
    d->garbage.assign( first, last );
  }

  
  void specification_processor::on_truth_table_line( unsigned line_index, const std::vector<boost::optional<bool> >::const_iterator first, const std::vector<boost::optional<bool> >::const_iterator last ) const
  {
    unsigned bw = last - first;

    binary_truth_table::cube_type in;
    binary_truth_table::cube_type out( first, last );

    for ( int pos = bw - 1; pos >= 0; --pos )
    {
      in.push_back( binary_truth_table::value_type( line_index & ( 1u << pos ) ) );
    }

    d->spec.add_entry( in, out );

    if ( !d->has_entries )
    {
      d->spec.set_constants( d->constants );
      d->spec.set_garbage( d->garbage );
      d->has_entries = true;
    }
  }

  bool read_specification( binary_truth_table& spec, std::istream& in, std::string* error )
  {
    specification_processor processor( spec );

    return revlib_parser( in, processor, revlib_parser_settings(), error );
  }

  bool read_specification( binary_truth_table& spec, const std::string& filename, std::string* error )
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

    return read_specification( spec, is, error );
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
