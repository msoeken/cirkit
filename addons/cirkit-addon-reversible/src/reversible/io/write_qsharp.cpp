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

#include "write_qsharp.hpp"

#include <fstream>

#include <boost/format.hpp>

#include <reversible/pauli_tags.hpp>
#include <reversible/target_tags.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

void write_gate( const gate& g, std::ostream& os )
{
  os << "            ";

  if ( is_toffoli( g ) )
  {
    if ( g.controls().size() == 0u )
    {
      os << boost::format( "X(qubits[%d]);" ) % g.targets().front() << std::endl;
    }
    else if ( g.controls().size() == 1u && g.controls().front().polarity() )
    {
      os << boost::format( "CNOT(qubits[%d], qubits[%d]);" ) % g.controls().front().line() % g.targets().front() << std::endl;
    }
    else
    {
      std::cerr << "[e] unsupported Toffoli gate" << std::endl;
      assert( false );
    }
  }
  else if ( is_hadamard( g ) )
  {
    os << boost::format( "H(qubits[%d]);" ) % g.targets().front() << std::endl;
  }
  else if ( is_pauli( g ) )
  {
    const auto pauli = boost::any_cast<pauli_tag>( g.type() );
    switch ( pauli.axis )
    {
    default:
      std::cerr << "[e] unsupported Pauli axis" << std::endl;
      assert( false );
      break;

    case pauli_axis::X:
      if ( pauli.root == 1 )
      {
        os << boost::format( "X(qubits[%d]);" ) % g.targets().front() << std::endl;
      }
      else
      {
        std::cerr << "[e] unsupported Pauli X root" << std::endl;
        assert( false );
      }
      break;

    case pauli_axis::Z:
      if ( pauli.root == 1 )
      {
        if ( g.controls().empty() )
        {
          os << boost::format( "Z(qubits[%d]);" ) % g.targets().front() << std::endl;
        }
        else if ( g.controls().size() == 1 && g.controls().front().polarity() )
        {
          os << boost::format( "CZ(qubits[%d], qubits[%d]);" ) % g.controls().front().line() % g.targets().front() << std::endl;
        }
        else
        {
          std::cerr << "[e] unsupported number of controls in Pauli Z gate" << std::endl;
          assert( false );
        }
      }
      else if ( pauli.root == 4 )
      {
        if ( pauli.adjoint )
        {
          os << boost::format( "(Adjoint T)(qubits[%d]);" ) % g.targets().front() << std::endl;
        }
        else
        {
          os << boost::format( "T(qubits[%d]);" ) % g.targets().front() << std::endl;
        }
      }
      else
      {
        std::cerr << "[e] unsupported Pauli Z root" << std::endl;
        assert( false );
      }
      break;
    }
  }
  else
  {
    std::cerr << "[e] unsupported gate" << std::endl;
    assert( false );
  }
}

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void write_qsharp( const circuit& circ, std::ostream& os, const properties::ptr& settings )
{
  using namespace std::string_literals;

  const auto namespace_name = get( settings, "namespace_name", "RevKit.Compilation"s );
  const auto operation_name = get( settings, "operation_name", "Oracle"s );

  os << boost::format( "namespace %s {" ) % namespace_name << std::endl
     << "    open Microsoft.Quantum.Primitive;" << std::endl
     << "    open Microsoft.Quantum.Canon;" << std::endl << std::endl;

  os << boost::format( "    operation %s(qubits : Qubit[]) : () {" ) % operation_name << std::endl
     << "        body {" << std::endl;

  for ( const auto& g : circ )
  {
    write_gate( g, os );
  }

  os << "        }" << std::endl
     << "        adjoint auto" << std::endl
     << "        controlled auto" << std::endl
     << "        controlled adjoint auto" << std::endl
     << "    }" << std::endl
     << "}" << std::endl;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void write_qsharp( const circuit& circ, const std::string& filename, const properties::ptr& settings )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  write_qsharp( circ, os, settings );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
