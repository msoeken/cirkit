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

#include "flow_scene.hpp"

#include <QtGui/QPen>
#include <QtWidgets/QAction>

#include "port_item.hpp"

FlowScenePortConnecter::FlowScenePortConnecter( QObject *parent ) : QObject( parent )
{
  parent->installEventFilter( this );

  // TODO
}

FlowSceneGraph::FlowSceneGraph( QObject *parent ) : QObject( parent )
{
  // TODO
}

FlowScene::FlowScene( QObject * parent ) : QGraphicsScene( parent )
{
  addRect( 0, 0, 1, 1, QPen( Qt::white ) );

  graph = new FlowSceneGraph( this );
  connecter = new FlowScenePortConnecter( this );

  // TODO

  setupActions();
}

void FlowScene::setupActions()
{
  loadAction = new QAction( QIcon::fromTheme( "document-open" ), "Open", this );
  saveAction = new QAction( QIcon::fromTheme( "document-save" ), "Save", this );
  saveAsAction = new QAction( QIcon::fromTheme( "document-save-as" ), "Save As", this );
  runAction = new QAction( QIcon::fromTheme( "run-build" ), "Run", this );

  connect( loadAction, SIGNAL( triggered() ), this, SLOT( slotLoad() ) );
  connect( saveAction, SIGNAL( triggered() ), this, SLOT( slotSave() ) );
  connect( saveAsAction, SIGNAL( triggered() ), this, SLOT( slotSaveAs() ) );
  connect( runAction, SIGNAL( triggered() ), this, SLOT( run() ) );
}

void FlowScene::slotLoad()
{
  // TODO
}

void FlowScene::slotSave()
{
  // TODO
}

void FlowScene::slotSaveAs()
{
  // TODO
}

void FlowScene::run()
{
  // TODO
}

#include "programs/gui/core/flow_scene.moc"

// Local Variables:
// c-basic-offset: 2
// End:
