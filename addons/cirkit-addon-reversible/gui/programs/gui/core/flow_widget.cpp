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

#include "flow_widget.hpp"
#include "flow_scene.hpp"

FlowWidget::FlowWidget( QWidget * parent ) : QGraphicsView( parent )
{
  setAcceptDrops( true );
  setScene( new FlowScene( this ) );
  setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );
  setAlignment( Qt::AlignLeft | Qt::AlignTop );

  // TODO

  setupMenu();
}

void FlowWidget::setupMenu()
{
  setContextMenuPolicy( Qt::CustomContextMenu );
  // TODO
  connect( this, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT(slotContextMenu( QPoint ) ) );
}

void FlowWidget::slotContextMenu( QPoint pos )
{
  // TODO
}

void FlowWidget::dragEnterEvent( QDragEnterEvent * event )
{
  // TODO
}

void FlowWidget::dragMoveEvent( QDragMoveEvent * event )
{
  // TODO
}

void FlowWidget::dropEvent( QDropEvent * event )
{
  // TODO
}

// Local Variables:
// c-basic-offset: 2
// End:

