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

#include "write_qiskit.hpp"

#include <fstream>

#include <reversible/pauli_tags.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/utils/circuit_utils.hpp>

#include <fmt/format.h>

namespace cirkit
{

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void write_x( std::ostream& os, const gate& g )
{
  if ( g.controls().empty() )
  {
    os << fmt::format( "qc.x(qr[{}])\n", g.targets().front() );
  }
  else if ( g.controls().size() == 1 && g.controls()[0].polarity() )
  {
    os << fmt::format( "qc.cx(qr[{}], qr[{}])\n", g.controls()[0].line(), g.targets().front() );
  }
  else if ( g.controls().size() == 2 && g.controls()[0].polarity() && g.controls()[1].polarity() )
  {
    os << fmt::format( "qc.ccx(qr[{}], qr[{}], qr[{}])\n", g.controls()[0].line(), g.controls()[1].line(), g.targets().front() );
  }
  else
  {
    os << "# Toffoli gate must have at most 2 positive controls\n";
  }
}

void write_z( std::ostream& os, const gate& g )
{
  if ( g.controls().empty() )
  {
    os << fmt::format( "qc.z(qr[{}])\n", g.targets().front() );
  }
  else if ( g.controls().size() == 1 && g.controls()[0].polarity() )
  {
    os << fmt::format( "qc.cz(qr[{}], qr[{}])\n", g.controls()[0].line(), g.targets().front() );
  }
  else
  {
    os << "# Z gate must have at most 1 positive control\n";
  }
}

void write_swap( std::ostream& os, const gate& g )
{
  if ( g.controls().size() == 0)
  {
    os << fmt::format( "qc.swap(qr[{}], qr[{}])\n", g.targets()[0], g.targets()[1] );
  }
  else if ( g.controls().size() == 1 && g.controls()[0].polarity() )
  {
    os << fmt::format( "qc.cswap(qr[{}], qr[{}], qr[{}])\n", g.controls()[0].line(), g.targets()[0], g.targets()[1] );
  }
  else
  {
    os << "# Fredkin gate must have at most 1 positive control";
  }
}

void write_qiskit( const circuit& circ, std::ostream& os, const properties::ptr& settings )
{
  const auto n = circ.lines();

  for ( const auto& g : circ )
  {
    if ( is_toffoli( g ) )
    {
      write_x( os, g );
    }
    else if ( is_fredkin( g ) )
    {
      write_swap( os, g );
    }
    else if ( is_hadamard( g ) )
    {
      if ( g.controls().size() == 0 )
      {
        os << fmt::format( "qc.h(qr[{}])\n", g.targets().front() );
      }
      else
      {
        os << "# Hadamard cannot have controls";
      }
    }
    else if ( is_pauli( g ) )
    {
      const auto pauli = boost::any_cast<pauli_tag>( g.type() );
      switch ( pauli.axis )
      {
      case pauli_axis::X:
        if ( pauli.root == 1u )
        {
          write_x( os, g );
        }
        else
        {
          os << "# unsupported X root\n";
        }
        break;
      case pauli_axis::Z:
        if ( pauli.root == 4u && g.controls().empty() )
        {
          os << fmt::format( "qc.{}(qr[{}])\n", ( pauli.adjoint ? "tdg" : "t" ), g.targets().front() );
        }
        else if ( pauli.root == 1u )
        {
          write_z( os, g );
        }
        else
        {
          os << "# unsupported Z root\n";
        }
        break;
      default:
        os << "# unsupported Pauli axis\n";
        break;
      }
    }
    else
    {
      os << "# unsupported gate\n";
    }
  }

  os << std::flush;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void write_qiskit( const circuit& circ, const std::string& filename, const properties::ptr& settings )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );

  write_qiskit( circ, os, settings );

  os.close();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
