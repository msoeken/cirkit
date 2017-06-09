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

#include "write_numpy.hpp"

#include <fstream>

#include <boost/algorithm/string/join.hpp>
#include <boost/format.hpp>

#include <reversible/pauli_tags.hpp>
#include <reversible/target_tags.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

std::string identity_padding( const std::string& matrix, unsigned from, unsigned to, unsigned lines )
{
  std::vector<std::string> gates;

  if ( from > 0u )
  {
    gates.push_back( boost::str( boost::format( "np.identity(%d)" ) % ( 1 << from ) ) );
  }

  gates.push_back( matrix );

  if ( to + 1u < lines )
  {
    gates.push_back( boost::str( boost::format( "np.identity(%d)" ) % ( 1 << ( lines - to - 1u ) ) ) );
  }

  switch ( gates.size() )
  {
  case 1u:
    return gates.front();

  case 2u:
    return boost::str( boost::format( "np.kron(%s, %s)" ) % gates[0u] % gates[1u] );

  default:
    return boost::str( boost::format( "nkron(%s)" ) % boost::join( gates, ", " ) );
  }
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void write_numpy( const circuit& circ, std::ostream& os )
{
  const auto n = circ.lines();

  os << "#!/usr/bin/env python3" << std::endl << std::endl;
  os << "import numpy as np" << std::endl;
  os << "from functools import reduce" << std::endl << std::endl;

  os << "X = np.array([[0, 1], [1, 0]])" << std::endl;
  os << "H = 1 / np.sqrt(2) * np.array([[1, 1], [1, -1]])" << std::endl;
  os << "T = np.array([[1, 0], [0, np.exp(1j * np.pi / 4)]])" << std::endl;
  os << "Tdag = np.array([[1, 0], [0, -np.exp(1j * np.pi / 4)]])" << std::endl;
  os << "zc = np.array([[1, 0], [0, 0]])" << std::endl;
  os << "oc = np.array([[0, 0], [0, 1]])" << std::endl;
  os << std::endl;

  os << "def nkron(*args):" << std::endl;
  os << "  return reduce(np.kron, args)" << std::endl;
  os << std::endl;

  os << "gates = []" << std::endl << std::endl;

  for ( const auto& g : circ )
  {
    const auto target = g.targets().front();

    if ( is_toffoli( g ) && g.controls().size() == 1u && g.controls().front().polarity() )
    {
      const auto control = g.controls().front().line();

      if ( control < target )
      {
        const auto act = target - control;
        const auto gate = boost::str( boost::format( "np.kron(zc, np.identity(%d)) + nkron(oc, np.identity(%d), X)" ) % ( 1 << act ) % ( 1 << ( act - 1 ) ) );
        os << boost::format( "gates.append(%s)" ) % identity_padding( gate, control, target, n ) << std::endl;
      }
      else
      {
        const auto act = control - target;
        const auto gate = boost::str( boost::format( "np.kron(np.identity(%d), zc) + nkron(X, np.identity(%d), oc)" ) % ( 1 << act ) % ( 1 << ( act - 1 ) ) );
        os << boost::format( "gates.append(%s)" ) % identity_padding( gate, target, control, n ) << std::endl;
      }
    }
    else if ( is_hadamard( g ) )
    {
      os << boost::format( "gates.append(%s)" ) % identity_padding( "H", target, target, n ) << std::endl;
    }
    else if ( is_pauli( g ) )
    {
      const auto pauli = boost::any_cast<pauli_tag>( g.type() );
      switch ( pauli.axis )
      {
      case pauli_axis::Z:
        if ( pauli.root == 4u )
        {
          os << boost::format( "gates.append(%s)" ) % identity_padding( pauli.adjoint ? "Tdag" : "T", target, target, n ) << std::endl;
        }
        else
        {
          os << "# unsupported gate" << std::endl;
        }
        break;
      default:
        os << "# unsupported gate" << std::endl;
        break;
      }
    }
    else
    {
      os << "# unsupported gate" << std::endl;
    }
  }

  os << std::endl;
  os << "circuit = reduce(np.dot, reversed(gates))" << std::endl;
  os << "print(np.round(circuit))" << std::endl;
}

void write_numpy( const circuit& circ, const std::string& filename )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );

  write_numpy( circ, os );

  os.close();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
