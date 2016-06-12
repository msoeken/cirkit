/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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
