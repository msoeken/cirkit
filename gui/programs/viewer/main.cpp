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

#include <QtWidgets/QApplication>

#include <core/circuit.hpp>
#include <core/io/read_realization.hpp>

#include <src/circuit_view.hpp>
#include "main_window.hpp"

using namespace revkit;

int main( int argc, char ** argv )
{
  QApplication app( argc, argv );

  circuit circ;
  read_realization( circ, "../../ext/circuits/3_17_13.real" );

  MainWindow window;
  window.circuitView()->load( circ );
  window.show();

  return app.exec();
}

// Local Variables:
// c-basic-offset: 2
// End:
