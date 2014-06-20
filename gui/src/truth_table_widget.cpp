/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2014  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include "truth_table_widget.hpp"

#include <boost/range/algorithm.hpp>
#include <boost/range/irange.hpp>

#include <reversible/truth_table.hpp>
#include <reversible/functions/circuit_to_truth_table.hpp>
#include <reversible/simulation/simple_simulation.hpp>

using namespace cirkit;

TruthTableWidget::TruthTableWidget( const circuit& circ, QWidget * parent )
  : QTableWidget( (1u << circ.lines() ), 2u * circ.lines(), parent )
{
  binary_truth_table spec;
  circuit_to_truth_table( circ, spec, simple_simulation_func() );

  QStringList hlabels, vlabels;

  boost::for_each( spec.inputs(), [&hlabels]( const std::string& i ) { hlabels.append( QString::fromStdString( i ) ); } );
  boost::for_each( spec.outputs(), [&hlabels]( const std::string& o ) { hlabels.append( QString::fromStdString( o ) ); } );
  boost::for_each( boost::irange( 0u, 1u << spec.num_inputs() ), [&vlabels]( unsigned n ) { vlabels.append( QString::number( n ) ); } );

  setHorizontalHeaderLabels( hlabels );
  setVerticalHeaderLabels( vlabels );
  setAlternatingRowColors( true );
  setShowGrid( false );

  unsigned row = 0u;
  for ( const auto& line : spec )
  {
    bool valid = true;
    for ( unsigned i = 0u; i < spec.num_inputs(); ++i )
    {
      // TODO add circuit as reference
    }

    unsigned col = 0u;
    for ( const auto& in : boost::make_iterator_range( line.first ) )
    {
      addItem( row, col, *in, !circ.constants()[col] || *circ.constants()[col] == *in );
      col++;
    }
    for ( const auto& out : boost::make_iterator_range( line.second ) )
    {
      addItem( row, col++, *out, !circ.garbage()[col - circ.lines()] );
    }

    ++row;
  }

  resizeColumnsToContents();
}

void TruthTableWidget::addItem( unsigned row, unsigned column, bool value, bool enabled )
{
  QTableWidgetItem * item = new QTableWidgetItem( value ? "1" : "0" );
  int flags = Qt::ItemIsSelectable;
  if ( enabled )
  {
    flags |= Qt::ItemIsEnabled;
  }
  item->setFlags( (Qt::ItemFlag)flags );
  setItem( row, column, item );
}

#include "src/truth_table_widget.moc"

// Local Variables:
// c-basic-offset: 2
// End:
