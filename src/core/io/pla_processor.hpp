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
 * @file pla_processor.hpp
 *
 * @brief Processor which works with the pla_parser
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef PLA_PROCESSOR_HPP
#define PLA_PROCESSOR_HPP

#include <iosfwd>
#include <vector>

namespace cirkit
{

  /**
   * @brief Base class for actions on the pla_parser
   *
   * @since  2.0
   */
  class pla_processor
  {
  public:
      virtual void on_comment( const std::string& comment ) {}
      virtual void on_num_inputs( unsigned num_inputs ) {}
      virtual void on_num_outputs( unsigned num_outputs ) {}
      virtual void on_num_products( unsigned num_products ) {}
      virtual void on_input_labels( const std::vector<std::string>& input_labels ) {}
      virtual void on_output_labels( const std::vector<std::string>& output_labels ) {}
      virtual void on_end() {}
      virtual void on_type( const std::string& type ) {}
      virtual void on_cube( const std::string& in, const std::string& out ) {}
  };
}

#endif /* PLA_PROCESSOR_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
