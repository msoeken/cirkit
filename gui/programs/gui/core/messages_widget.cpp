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

#include "messages_widget.hpp"


MessagesWidget::MessagesWidget( QWidget * parent ) : QTreeWidget( parent )
{
  setHeaderLabels( { "Message", "Sender" } );
  setRootIsDecorated( false );
  setColumnWidth( 0, 700 );
}

void MessagesWidget::addMessage( const QString& message, const QString& sender, const QString& icon )
{
  auto * item = new QTreeWidgetItem( { message, sender } );
  item->setIcon( 0, QIcon::fromTheme( "dialog-" + icon ) );
  addTopLevelItem( item );
  emit messageAdded();
}

#include "programs/gui/core/messages_widget.moc"

// Local Variables:
// c-basic-offset: 2
// End:
