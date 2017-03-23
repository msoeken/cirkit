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

/**
 * @file terminal.hpp
 *
 * @brief Some helper functions for working with terminals
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <algorithm>
#include <iostream>
#include <string>

#include <boost/format.hpp>

namespace cirkit
{

void print_banner( const std::string& caption, unsigned width = 40u );

class null_stream : public std::streambuf
{
public:
  inline int overflow( int c ) { return c; }
};

/**
 * progress_line
 *
 * Over-writing single-line progress bar
 *
 * Typical usecase:
 *
 * Use a settings variable `progress' of type `bool' that is default
 * initialized to `true'.  Assume that you want to show progress in
 * the algorithm for three variables `aString', `aNumber', and
 * `aFloat'.  Then you get a progress line as follows:
 *
 *     progress_line p( "[i] s = %s   n = %d    f = %f", progress );
 *
 *     while ( ... )
 *     {
 *       p( aString, aNumber, aFloat );
 *     }
 *
 * The deconstructor of progress_line will clear the line by printing
 * as many spaces as the maximum line processed.  Make sure not to
 * have line breaks in the string passed to the constructor, and not
 * to have any writes to the output stream passed to progress_line.
 */
class progress_line
{
public:
  progress_line( const std::string& format, bool enable = true, std::ostream& os = std::cout );
  ~progress_line();

  inline void keep_last() { _keep_last = true; }
  void clear();

  template<typename... Args>
  void operator()( Args&&... args )
  {
    if ( !enable ) return;

    boost::format fmt( format );
    using unroll = int[];
    static_cast<void>( unroll{0, ( fmt % std::forward<Args>( args ), 0)...} );

    last_string = boost::str( fmt );
    max_length = std::max( last_string.size(), max_length );

    os << last_string << "\r";
    os.flush();
  }

  class subprogress_t
  {
  public:
    subprogress_t( bool enable, std::ostream& os ) : enable( enable ), os( os )
    {
      if ( enable )
      {
        os << "\n";
      }
    }

    ~subprogress_t()
    {
      if ( enable )
      {
        os << "\e[A";
      }
    }

  private:
    bool enable;
    std::ostream& os;
  };

  std::unique_ptr<subprogress_t> subprogress() const
  {
    return std::unique_ptr<subprogress_t>( new subprogress_t( enable, os ) );
  }

private:
  std::string format;
  bool enable;
  std::ostream& os;
  bool _keep_last = false;
  std::string last_string;
  std::size_t max_length = 0u;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
