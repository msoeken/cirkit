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

#include "flow_container.hpp"

#include <QtWidgets/QAction>

#include <programs/gui/core/flow_widget.hpp>

FlowContainer::FlowContainer( QWidget * parent ) : QTabWidget( parent )
{
  setDocumentMode( true );
  setTabsClosable( true );
  setupActions();

  connect( this, SIGNAL( tabCloseRequested(int) ), this, SLOT( slotClose(int) ) );
}

void FlowContainer::setupActions()
{
  newAction = new QAction( QIcon::fromTheme( "document-new" ), "New", this );
  openAction = new QAction( QIcon::fromTheme( "document-open" ), "Open", this );
  saveAction = new QAction( QIcon::fromTheme( "document-save" ), "Save", this );
  saveAsAction = new QAction( QIcon::fromTheme( "document-save-as" ), "Save As", this );
  runAction = new QAction( QIcon::fromTheme( "run-build" ), "Run", this );

  connect( newAction, SIGNAL( triggered() ), this, SLOT( slotNew() ) );
  connect( openAction, SIGNAL( triggered() ), this, SLOT( slotOpen() ) );
  connect( saveAction, SIGNAL( triggered() ), this, SLOT( slotSave() ) );
  connect( saveAsAction, SIGNAL( triggered() ), this, SLOT( slotSaveAs() ) );
  connect( runAction, SIGNAL( triggered() ), this, SLOT( slotRun() ) );
}

void FlowContainer::updateActions()
{
  saveAction->setEnabled( count() > 0u );
  saveAsAction->setEnabled( count() > 0u );
  runAction->setEnabled( count() > 0u );
}

void FlowContainer::slotNew()
{
  FlowWidget * widget = new FlowWidget( this );
  setCurrentIndex( addTab( widget, QIcon::fromTheme( "text-rdf+xml" ), "New Graph" ) );
  // TODO
}

void FlowContainer::slotOpen()
{
  // TODO
}

void FlowContainer::slotSave()
{
  // TODO
}

void FlowContainer::slotSaveAs()
{
  // TODO
}

void FlowContainer::slotRun()
{
  // TODO
}

void FlowContainer::slotClose( int index )
{
  // TODO
}

void FlowContainer::slotFilenameChanged()
{
  // TODO
}

void FlowContainer::slotModifiedChanged()
{
  // TODO
}

#include "programs/gui/core/flow_container.moc"

// Local Variables:
// c-basic-offset: 2
// End:
