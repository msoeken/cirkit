/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include <iostream>

#include <core/io/read_pla_to_bdd.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/functions/extend_pla.hpp>
#include <reversible/io/read_pla.hpp>

using namespace revkit;

int main( int argc, char ** argv )
{
  // usage
  if ( argc != 2 )
  {
    std::cout << "usage: " << argv[0] << " filename" << std::endl;
    return 1;
  }

  binary_truth_table pla, extended;
  read_pla_settings settings;
  settings.extend = false;
  read_pla( pla, argv[1], settings );

  std::cout << pla << std::endl;

  extend_pla( pla, extended );

  std::cout << "Extended PLA:" << std::endl;
  std::cout << extended << std::endl;

  BDDTable base;
  read_pla_to_bdd( base, argv[1] );

  return 0;
}
