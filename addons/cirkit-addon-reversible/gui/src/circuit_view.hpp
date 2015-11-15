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

#ifndef CIRCUIT_VIEW_HPP
#define CIRCUIT_VIEW_HPP

#include <memory>

#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QWidget>

#include <reversible/circuit.hpp>

class QGraphicsTextItem;

class CircuitView : public QGraphicsView
{
  Q_OBJECT

public:
  explicit CircuitView( QWidget * parent = nullptr );

  void load( const std::shared_ptr<cirkit::circuit>& circ );
  void saveImage( const QString& filename ) const;

private:
  void setupActions();
  void setupContextMenu();

  QGraphicsTextItem * addLineLabel( int x, int y, QString text, unsigned align, bool color );

private Q_SLOTS:
  void copyLatexToClipboard();
  void showContextMenu( const QPoint& pos );

private:
  class Private;
  Private * const d;
};

#endif

// Local Variables:
// c-basic-offset: 2
// End:
