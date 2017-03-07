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

#include "demo.hpp"

#include <iostream>

#include <boost/format.hpp>

#include <classical/xmg/xmg.hpp>    /* XMG data structure */

namespace cirkit
{

/******************************************************************************
 * XMG demo                                                                   *
 ******************************************************************************/

void xmg_demo( bool verbose )
{
  /* create a new empty XMG */
  xmg_graph xmg;

  /* the plan is to create a full adder, first we need three inputs */
  const auto a   = xmg.create_pi( "a" );
  const auto b   = xmg.create_pi( "b" );
  const auto cin = xmg.create_pi( "cin" );

  /* now we create the expressions */
  const auto sum  = xmg.create_xor( xmg.create_xor( a, b ), cin );
  const auto cout = xmg.create_maj( a, b, cin );

  /* sum and cout are values of type xmg_function. These represent pointers
   * to node with a complement flag.  We need to make them outputs of the
   * XMG explicitly
   */
  xmg.create_po( sum, "sum" );
  xmg.create_po( cout, "cout" );

  /* the XMG should now have 7 nodes, which are 3 inputs, 1 MAJ gate, 2 XOR gates,
   * and also one constant 0 node, which is always there, even if we don't use it;
   * let's print some statistics
   */
  if ( verbose )
  {
    std::cout << "[i] number of nodes:   " << xmg.size() << std::endl
              << "[i] number of inputs:  " << xmg.inputs().size() << std::endl
              << "[i] number of outputs: " << xmg.outputs().size() << std::endl
              << "[i] number of MAJ:     " << xmg.num_maj() << std::endl
              << "[i] number of XOR:     " << xmg.num_xor() << std::endl
              << "[i] number of gates:   " << xmg.num_gates() << std::endl;
  }

  /* let's now print all the input and output names */
  if ( verbose )
  {
    /* xmg.inputs() returns pairs of input nodes (xmg_node) and names
     */
    std::cout << "[i] input names:      ";
    for ( const auto& input : xmg.inputs() )
    {
      std::cout << " " << input.second;
    }
    std::cout << std::endl;

    /* xmg.outputs() returns pairs of output pointers (xmg_function) and names
     */
    std::cout << "[i] output names:     ";
    for ( const auto& output : xmg.outputs() )
    {
      std::cout << " " << output.second;
    }
    std::cout << std::endl;
  }

  /* next we compute the level of each node.  This information is not directly
   * available, and we first must compute it with `compute_levels', afterwards
   * we can access it with `level'.
   */
  xmg.compute_levels();
  if ( verbose )
  {
    for ( auto node : xmg.nodes() )
    {
      std::cout << boost::format( "[i] node %d is at level %d" ) % node % xmg.level( node ) << std::endl;
    }
  }
}

/******************************************************************************
 * implementation of the command                                              *
 ******************************************************************************/

demo_command::demo_command( const environment::ptr& env )
  : cirkit_command( env, "Demonstrate some of the API" )
{
  /* Options can be added one by one using opts.add_options()
   * There are in principle 3 types of options:
   * - Boolean flags:
   *
   *   ( "long_name,f", "description")
   *
   *   Here long_name is passed as --long_name to the command
   *   the flag f is a shorthand one character alternative passed
   *   as -f . The flag is optional. You can check whether in
   *   option has been set in execute() function using
   *   is_set( "long_name" )
   *
   * - Values without default value:
   *
   *   ( "long_name,f", value( &variable ), "description" )
   *
   *   Here variable is a private parameter in the demo_command
   *   class. It can be any string representable type.  This
   *   interface is based on Boost.ProgramOptions. Check their
   *   documentation for details.  You can use the variable then
   *   directly in the execute() function, but you should check whether
   *   it has been set using is_set( "long_name" ) . In order to
   *   use value, you add these two lines to the top of the file:
   *
   *   #include <boost/program_options.hpp>
   *   using boost::program_options::value;
   *
   * - Values with default value:
   *
   *   ( "long_name,f", value_with_default( &variable ), "description" )
   *
   *   Like the case without default value, but make sure that your
   *   variable is assigned a default value when being declared in the
   *   class. Also add these two lines:
   *
   *   #include <core/utils/program_options.hpp>
   *
   *   And you don't need to check whether the variable is set before
   *   using it.
   */
  opts.add_options()
    ( "xmg,x", "demonstration of XMG features" )
    ;

  /* this adds a verbose option to the command */
  be_verbose();
}

bool demo_command::execute()
{
  if ( is_set( "xmg" ) )
  {
    xmg_demo( is_verbose() );
  }

  /* command should always return true, false leads to program termination */
  return true;
}


}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
