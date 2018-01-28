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

#include "write_projectq.hpp"

#include <fstream>

#include <boost/algorithm/string/join.hpp>
#include <boost/format.hpp>

#include <reversible/pauli_tags.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/utils/circuit_utils.hpp>

#include <fmt/format.h>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void write_controlled_gate( std::ostream& os, const std::string& indent, const gate& g, const std::string& gatename)
{
  std::string neg;
  std::vector<std::string> qubits;
  for ( const auto& c : g.controls() )
  {
    if ( !c.polarity() )
    {
      neg += fmt::format( "{}X | qubits[{}]\n", indent, c.line() );
    }
    qubits.push_back( fmt::format( "qubits[{}]", c.line() ) );
  }

  for ( auto t : g.targets() )
  {
    qubits.push_back( fmt::format( "qubits[{}]", t ) );
  }

  if ( g.controls().empty() )
  {
    os << fmt::format( "{}{} | ({})\n", indent, gatename, boost::join( qubits, ", " ) );
  }
  else
  {
    os << fmt::format( "{}{}C({}, {}) | ({})\n{}", neg, indent, gatename, g.controls().size(), boost::join( qubits, ", " ), neg );
  }
}

void write_projectq( const circuit& circ, std::ostream& os, const properties::ptr& settings )
{
  const auto check_identity = get( settings, "check_identity", true );
  const auto standalone = get( settings, "standalone", false );

  const auto n = circ.lines();
  if ( standalone )
  {
    os << "import itertools" << std::endl;
    os << "import sys" << std::endl << std::endl;

    os << "#!/usr/bin/env python3" << std::endl << std::endl;
    os << "import projectq" << std::endl;
    os << "from projectq.cengines import MainEngine" << std::endl;
    os << "from projectq.ops import H, C, CNOT, Swap, Toffoli, NOT, X, Measure, T, Tdag" << std::endl << std::endl;

    os << "import numpy as np" << std::endl << std::endl;

    os << "num_qubits = " << n << std::endl << std::endl;

    os << "def simulate(input):" << std::endl;
    os << "    eng = MainEngine()" << std::endl;
    os << "    qubits = eng.allocate_qureg(num_qubits)" << std::endl;
    os << "    for i, v in enumerate(input):" << std::endl;
    os << "        if v:" << std::endl;
    os << "            X | qubits[i]" << std::endl << std::endl;
  }

  std::string indent( ' ', standalone ? 4 : 0 );

  for ( const auto& g : circ )
  {
    const auto target = g.targets().front();

    if ( is_toffoli( g ) )
    {
      write_controlled_gate( os, indent, g, "NOT" );
    }
    else if ( is_fredkin( g ) )
    {
      write_controlled_gate( os, indent, g, "Swap" );
    }
    else if ( is_hadamard( g ) )
    {
      os << boost::format( "%sH | qubits[%d]" ) % indent % target << std::endl;
    }
    else if ( is_pauli( g ) )
    {
      const auto pauli = boost::any_cast<pauli_tag>( g.type() );
      switch ( pauli.axis )
      {
      case pauli_axis::X:
        if ( pauli.root == 1u )
        {
          write_controlled_gate( os, indent, g, "X" );
        }
        else
        {
          os << "# unsupported X root" << std::endl;
        }
        break;
      case pauli_axis::Z:
        if ( pauli.root == 4u )
        {
          os << boost::format( "%s%s | qubits[%d]" ) % indent % ( pauli.adjoint ? "Tdag" : "T" ) % target << std::endl;
        }
        else if ( pauli.root == 1u )
        {
          write_controlled_gate( os, indent, g, "Z" );
        }
        else
        {
          os << "# unsupported Z root" << std::endl;
        }
        break;
      default:
        os << "# unsupported Pauli axis" << std::endl;
        break;
      }
    }
    else
    {
      os << "# unsupported gate" << std::endl;
    }
  }

  if ( standalone )
  {
    os << "    eng.flush()" << std::endl << std::endl;

    os << "    r = [[eng.backend.get_amplitude(p, qubits) for p in itertools.product('01', repeat = num_qubits)]]" << std::endl << std::endl;

    os << "    Measure | qubits" << std::endl << std::endl;

    os << "    return r" << std::endl << std::endl;

    os << "M = np.concatenate([simulate(input) for input in itertools.product([False, True], repeat = num_qubits)], axis = 0)" << std::endl << std::endl;

    os << "np.set_printoptions(linewidth = 200)" << std::endl;

    if ( check_identity )
    {
      os << boost::format( "print(np.array_equal(np.round(M), np.identity(%d)))" ) % ( 1 << n ) << std::endl;
    }
    else
    {
      os << "print(np.round(M))" << std::endl;
    }
  }
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void write_projectq( const circuit& circ, const std::string& filename, const properties::ptr& settings )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );

  write_projectq( circ, os, settings );

  os.close();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
