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

#pragma once

#include <boost/any.hpp>

#include <QtWidgets/QGraphicsObject>

class PortItem : public QGraphicsObject
{
  Q_OBJECT

public:
  PortItem( const QString& datatype, int direction = Qt::AlignTop, QGraphicsItem * parent = nullptr );

  QRectF boundingRect() const;
  void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = nullptr );

  void setValue( boost::any value );
  boost::any value() const;

private:
  unsigned radius = 8u;
  QString datatype;
  int direction;
  int datatypeWidth, datatypeHeight;
  boost::any _value;
};

// Local Variables:
// c-basic-offset: 2
// End:
