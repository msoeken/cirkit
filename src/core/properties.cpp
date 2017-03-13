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

#include "properties.hpp"

namespace cirkit
{

  properties::properties()
  {
  }

  const properties::value_type& properties::operator[]( const properties::key_type& k ) const
  {
    return map.find( k )->second;
  }

  void properties::set( const properties::key_type& k, const properties::value_type& value )
  {
    map[k] = value;
  }

  properties::storage_type::const_iterator properties::begin() const
  {
    return map.begin();
  }

  properties::storage_type::const_iterator properties::end() const
  {
    return map.end();
  }

  unsigned properties::size() const
  {
    return map.size();
  }

  void properties::clear()
  {
    map.clear();
  }

  void set_error_message( properties::ptr statistics, const std::string& error )
  {
    if ( statistics )
    {
      statistics->set( "error", error );
    }
  }

  void make_settings_rec( const properties::ptr& settings )
  {
  }
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
