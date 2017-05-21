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

#include "write_qcode.hpp"

#include <fstream>

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

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void write_qcode( const circuit& circ, const std::string& filename )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  write_qcode( circ, os );
  os.close();
}

void write_qcode( const circuit& circ, std::ostream& os )
{
  os << "qubits " << circ.lines() << std::endl << std::endl;

  /* explicit constant initialization */
  bool explinit = false;
  for ( auto i = 0u; i < circ.lines(); ++i )
  {
    if ( circ.constants()[i] )
    {
      os << "prepz q" << i << std::endl;
      if ( *( circ.constants()[i] ) )
      {
        os << "x q" << i << std::endl;
      }
      explinit = true;
    }
  }
  if ( explinit )
  {
    os << std::endl;
  }

  for ( const auto& g : circ )
  {
    if ( is_toffoli( g ) )
    {
      const auto target = g.targets().front();
      switch ( g.controls().size() )
      {
      case 0u:
        os << "x q" << target << std::endl;
        break;
      case 1u:
        {
          const auto& c = g.controls().front();
          if ( !c.polarity() )
          {
            os << "[w] CNOT with negative control line unsupported, skipping" << std::endl;
          }
          else
          {
            os << boost::format( "cnot q%d, q%d" ) % c.line() % target << std::endl;
          }
        } break;
      case 2u:
        {
          const auto& c1 = g.controls()[0u];
          const auto& c2 = g.controls()[1u];
          if ( !c1.polarity() || !c2.polarity() )
          {
            os << "[w] Toffoli with negative control line unsupported, skipping" << std::endl;
          }
          else
          {
            os << boost::format( "toffoli q%d, q%d, q%d" ) % c1.line() % c2.line() % target << std::endl;
          }
        } break;
      default:
        os << "[w] Toffoli with more than 2 control lines unsupported, skipping" << std::endl;
        break;
      }
    }
    else if ( is_fredkin( g ) )
    {
      if ( !g.controls().empty() )
      {
        os << "[w] controlled SWAPs unsupported, skipping" << std::endl;
      }
      else
      {
        os << boost::format( "swap q%d, q%d" ) % g.targets()[0u] % g.targets()[1u] << std::endl;
      }
    }
    else if ( is_pauli( g ) )
    {
      const auto pauli = boost::any_cast<pauli_tag>( g.type() );
      switch ( pauli.axis )
      {
      case pauli_axis::X:
        if ( pauli.root > 1 )
        {
          os << boost::format( "[w] roots of Pauli-X unsupported, skipping" ) << std::endl;
        }
        else
        {
          os << "x q" << g.targets().front() << std::endl;
        }
        break;
      case pauli_axis::Y:
        if ( pauli.root > 1 )
        {
          os << boost::format( "[w] roots of Pauli-Y unsupported, skipping" ) << std::endl;
        }
        else
        {
          os << "y q" << g.targets().front() << std::endl;
        }
        break;
      case pauli_axis::Z:
        if ( pauli.root == 1 )
        {
          os << "z q" << g.targets().front() << std::endl;
        }
        else if ( pauli.root == 2 )
        {
          os << boost::format( "s q%d" ) % g.targets().front() << std::endl;
        }
        else if ( pauli.root == 4 )
        {
          os << boost::format( "t%s q%d" ) % ( pauli.adjoint ? "dag" : "" ) % g.targets().front() << std::endl;
        }
        else
        {
          os << boost::format( "[w] only first, second, and fourth root of Pauli-Z supported, skipping" ) << std::endl;
        }
        break;
      }
    }
    else if ( is_hadamard( g ) )
    {
      os << "h q" << g.targets().front() << std::endl;
    }
    else
    {
      std::cout << "[w] unsupported gate type to write qcode, skipping" << std::endl;
    }
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
