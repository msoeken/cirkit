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

#include "read_pla.hpp"

#include <vector>

#include <cuddObj.hh>
#include <cuddInt.h>

#include <core/io/pla_parser.hpp>
#include <core/io/pla_processor.hpp>
#include <core/utils/range_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

class read_pla_to_cudd_processor : public pla_processor
{
public:
  read_pla_to_cudd_processor( Cudd& manager ) : manager( manager ) {}

  void on_num_inputs( unsigned num_inputs )
  {
    ntimes( num_inputs, [&]() { manager.bddVar(); } );
  }

  void on_num_outputs( unsigned num_outputs )
  {
    functions.resize( num_outputs, manager.bddZero() );
  }

  void on_cube( const std::string& in, const std::string& out )
  {
    auto cube = manager.bddOne();

    for ( auto c : index( in ) )
    {
      switch ( c.value )
      {
      case '0':
        cube &= !manager.bddVar( c.index );
        break;
      case '1':
        cube &= manager.bddVar( c.index );
        break;
      }
    }

    for ( auto c : index( out ) )
    {
      if ( c.value == '1' )
      {
        functions[c.index] |= cube;
      }
    }
  }

public:
  Cudd&            manager;
  std::vector<BDD> functions;
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

bdd_function_t read_pla( const std::string& filename )
{
  Cudd manager;

  read_pla_to_cudd_processor p( manager );
  pla_parser( filename, p );

  return {manager, p.functions};
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
