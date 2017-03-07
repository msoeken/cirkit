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

#include "revlib_processor.hpp"

#include <iostream>

namespace cirkit
{

  ////////////////////////////// class revlib_processor
  class revlib_processor::priv
  {
  public:
    priv()
      : current_annotations( new properties() ) {}

    std::vector<std::string> vars;
    properties::ptr current_annotations;
  };

  revlib_processor::revlib_processor()
    : d( new priv() )
  {
  }

  revlib_processor::~revlib_processor()
  {
    delete d;
  }

  void revlib_processor::on_comment( const std::string& comment ) const
  {
  }

  void revlib_processor::on_version( const std::string& version ) const
  {
  }

  void revlib_processor::on_numvars( unsigned numvars ) const
  {
  }

  void revlib_processor::on_variables( std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last ) const
  {
    d->vars.clear();
    std::copy( first, last, std::back_inserter( d->vars ) );
  }

  void revlib_processor::on_inputs( std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last ) const
  {
  }

  void revlib_processor::on_outputs( std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last ) const
  {
  }

  void revlib_processor::on_constants( std::vector<constant>::const_iterator first, std::vector<constant>::const_iterator last ) const
  {
  }

  void revlib_processor::on_garbage( std::vector<bool>::const_iterator first, std::vector<bool>::const_iterator last ) const
  {
  }

  void revlib_processor::on_inputbus( const std::string& name, const std::vector<unsigned>& line_indices ) const
  {
  }

  void revlib_processor::on_outputbus( const std::string& name, const std::vector<unsigned>& line_indices ) const
  {
  }

  void revlib_processor::on_state( const std::string& name, const std::vector<unsigned>& line_indices, unsigned initial_value ) const
  {
  }

  void revlib_processor::on_module( const std::string& name, const boost::optional<std::string>& filename ) const
  {
  }

  void revlib_processor::on_begin() const
  {
  }

  void revlib_processor::on_end() const
  {
  }

  void revlib_processor::on_gate( const boost::any& target_type, const std::vector<variable>& line_indices ) const
  {
  }

  void revlib_processor::on_truth_table_line( unsigned line_index, const std::vector<boost::optional<bool> >::const_iterator first, const std::vector<boost::optional<bool> >::const_iterator last ) const
  {
  }

  void revlib_processor::add_annotation( const std::string& key, const std::string& value )
  {
    d->current_annotations->set( key, value );
  }

  void revlib_processor::clear_annotations()
  {
    d->current_annotations->clear();
  }

  properties::ptr revlib_processor::current_annotations() const
  {
    return d->current_annotations;
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
