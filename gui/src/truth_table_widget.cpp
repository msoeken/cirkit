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

#include "truth_table_widget.hpp"

#include <boost/range/algorithm.hpp>
#include <boost/range/irange.hpp>

using namespace revkit;

TruthTableWidget::TruthTableWidget( const binary_truth_table& spec, QWidget * parent )
  : QTableWidget( (1u << spec.num_inputs() ), 2u * spec.num_inputs(), parent )
{
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
      QTableWidgetItem * item = new QTableWidgetItem( *in ? "1" : "0" );
      setItem( row, col++, item );
    }
    for ( const auto& out : boost::make_iterator_range( line.second ) )
    {
      QTableWidgetItem * item = new QTableWidgetItem( *out ? "1" : "0" );
      setItem( row, col++, item );
    }

    ++row;
  }

  resizeColumnsToContents();
}

#include "src/truth_table_widget.moc"

// Local Variables:
// c-basic-offset: 2
// End:
