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
