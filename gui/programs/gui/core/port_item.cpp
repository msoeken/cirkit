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

#include "port_item.hpp"

#include <QtGui/QPainter>

PortItem::PortItem(const QString &datatype, int direction, QGraphicsItem *parent)
  : QGraphicsObject( parent ),
    datatype( datatype ), direction( direction )
{
  setCacheMode( QGraphicsItem::DeviceCoordinateCache );
}

QRectF PortItem::boundingRect() const
{
  int y = direction == Qt::AlignTop ? -radius / 2 : -radius / 2 - 20 - datatypeHeight;
  return QRectF( -datatypeWidth / 2, y, datatypeWidth, radius + 20 + datatypeHeight);
}

void PortItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  painter->setBrush( QColor( Qt::black ).lighter() );
  painter->setPen( Qt::black );
  painter->drawRect( QRectF( -radius / 2, -radius / 2, radius, radius ) );
  datatypeWidth = painter->fontMetrics().width( datatype );
  datatypeHeight = painter->fontMetrics().height();
  int y = direction == Qt::AlignTop ? radius / 2 - datatypeHeight : radius + 8;
  painter->drawText( datatypeWidth / 2.0, y, datatype );
}

#include "programs/gui/core/port_item.moc"

// Local Variables:
// c-basic-offset: 2
// End:
