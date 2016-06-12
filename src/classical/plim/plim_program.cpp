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

#include "plim_program.hpp"

#include <boost/format.hpp>

#include <core/utils/range_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

std::string register_string( memristor_index reg )
{
  return boost::str( boost::format( "@X%d" ) % reg.index() );
}

struct operand_visitor : public boost::static_visitor<std::string>
{
  std::string operator()( bool value ) const
  {
    return value ? "true" : "false";
  }

  std::string operator()( memristor_index reg ) const
  {
    return register_string( reg );
  }
};

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void plim_program::read_constant( memristor_index dest, bool value )
{
  /* !value -> 0 1 dest
      value -> 1 0 dest */
  compute( dest, false != value, true != value );
}

void plim_program::invert( memristor_index dest, memristor_index src )
{
  read_constant( dest, false );
  compute( dest, true, src );
}

void plim_program::assign( memristor_index dest, memristor_index src )
{
  read_constant( dest, true );
  compute( dest, src, true );
}

void plim_program::compute( memristor_index dest, operand_t src_pos, operand_t src_neg )
{
  _instructions.push_back( std::make_tuple( src_pos, src_neg, dest ) );

  auto index = dest.index() - 1u;
  if ( index >= _write_counts.size() )
  {
    _write_counts.resize( index + 1, 0u );
  }

  _write_counts[index]++;
}

const std::vector<plim_program::instruction_t>& plim_program::instructions() const
{
  return _instructions;
}

unsigned plim_program::step_count() const
{
  return _instructions.size();
}

unsigned plim_program::rram_count() const
{
  return _write_counts.size();
}

const std::vector<unsigned>& plim_program::write_counts() const
{
  return _write_counts;
}

std::ostream& operator<<( std::ostream& os, const plim_program& program )
{
  operand_visitor vis;

  auto cnt = 0u;
  for ( const auto& i : program.instructions() )
  {
    os << boost::format( "%04d: %8s, %8s, %8s" ) % ++cnt %
              boost::apply_visitor( vis, std::get<0>( i ) ) %
              boost::apply_visitor( vis, std::get<1>( i ) ) %
              register_string( std::get<2>( i ) )
       << std::endl;
  }

  return os;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
