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
 * @file plim_program.hpp
 *
 * @brief PLiM program
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef PLIM_PROGRAM_HPP
#define PLIM_PROGRAM_HPP

#include <iostream>

#include <boost/variant.hpp>

#include <core/utils/index.hpp>

namespace cirkit
{

struct memristor_index_tag;
using memristor_index = base_index<memristor_index_tag>;

class plim_program
{
public:
  using operand_t     = boost::variant<memristor_index, bool>;
  using instruction_t = std::tuple<operand_t, operand_t, memristor_index>;

public:
  void read_constant( memristor_index dest, bool value );
  void invert( memristor_index dest, memristor_index src );
  void assign( memristor_index dest, memristor_index src );
  void compute( memristor_index dest, operand_t src_pos, operand_t src_neg );

  const std::vector<instruction_t>& instructions() const;

  unsigned step_count() const;
  unsigned rram_count() const;
  const std::vector<unsigned>& write_counts() const;

private:
  std::vector<instruction_t> _instructions;
  std::vector<unsigned>      _write_counts;
};

std::ostream& operator<<( std::ostream& os, const plim_program& program );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
