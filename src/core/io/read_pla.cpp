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
