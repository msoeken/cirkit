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

#include "write_pyquil.hpp"

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

inline void pyquil_write_x( std::ostream& os, const gate& g )
{
  if ( g.controls().empty() )
  {
    os << fmt::format( "p.inst(X(qr[{}]))\n", g.targets().front() );
  }
  else if ( g.controls().size() == 1 && g.controls()[0].polarity() )
  {
    os << fmt::format( "p.inst(CNOT(qr[{}], qr[{}]))\n", g.controls()[0].line(), g.targets().front() );
  }
  else if ( g.controls().size() == 2 && g.controls()[0].polarity() && g.controls()[1].polarity() )
  {
    os << fmt::format( "p.inst(CCNOT(qr[{}], qr[{}], qr[{}]))\n", g.controls()[0].line(), g.controls()[1].line(), g.targets().front() );
  }
  else
  {
    os << "raise RuntimeError('Toffoli gate must have at most 2 positive controls')\n";
  }
}

inline void pyquil_write_z( std::ostream& os, const gate& g )
{
  if ( g.controls().empty() )
  {
    os << fmt::format( "p.inst(Z(qr[{}]))\n", g.targets().front() );
  }
  else if ( g.controls().size() == 1 && g.controls()[0].polarity() )
  {
    os << fmt::format( "p.inst(CZ(qr[{}], qr[{}]))\n", g.controls()[0].line(), g.targets().front() );
  }
  else
  {
    os << "raise RuntimeError('Z gate must have at most 1 positive control')\n";
  }
}

inline void pyquil_write_swap( std::ostream& os, const gate& g )
{
  if ( g.controls().size() == 0)
  {
    os << fmt::format( "p.inst(SWAP(qr[{}], qr[{}]))\n", g.targets()[0], g.targets()[1] );
  }
  else if ( g.controls().size() == 1 && g.controls()[0].polarity() )
  {
    os << fmt::format( "p.inst(CSWAP(qr[{}], qr[{}], qr[{}]))\n", g.controls()[0].line(), g.targets()[0], g.targets()[1] );
  }
  else
  {
    os << "raise RuntimeError('Fredkin gate must have at most 1 positive control')\n";
  }
}

void write_pyquil( const circuit& circ, std::ostream& os, const properties::ptr& settings )
{
  const auto n = circ.lines();

  for ( const auto& g : circ )
  {
    if ( is_toffoli( g ) )
    {
      pyquil_write_x( os, g );
    }
    else if ( is_fredkin( g ) )
    {
      pyquil_write_swap( os, g );
    }
    else if ( is_hadamard( g ) )
    {
      if ( g.controls().size() == 0 )
      {
        os << fmt::format( "p.inst(H(qr[{}]))\n", g.targets().front() );
      }
      else
      {
        os << "raise RuntimeError('Hadamard cannot have controls')\n";
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
          pyquil_write_x( os, g );
        }
        else
        {
          os << "raise RuntimeError('unsupported X root')\n";
        }
        break;
      case pauli_axis::Z:
        if ( pauli.root == 4u && g.controls().empty() && !pauli.adjoint )
        {
          os << fmt::format( "p.inst(T(qr[{}]))\n", g.targets().front() );
        }
        else if ( pauli.root == 1u )
        {
          pyquil_write_z( os, g );
        }
        else
        {
          os << "raise RuntimeError('unsupported Z root')\n";
        }
        break;
      default:
        os << "raise RuntimeError('unsupported Pauli axis')\n";
        break;
      }
    }
    else
    {
      os << "raise RuntimeError('unsupported gate')\n";
    }
  }

  os << std::flush;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void write_pyquil( const circuit& circ, const std::string& filename, const properties::ptr& settings )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );

  write_pyquil( circ, os, settings );

  os.close();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
