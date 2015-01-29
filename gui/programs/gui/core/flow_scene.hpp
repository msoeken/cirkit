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

#include <QtWidgets/QGraphicsScene>

#include <boost/optional.hpp>

class QAction;

class PortItem;

class FlowScenePortConnecter : public QObject
{
  Q_OBJECT

public:
  FlowScenePortConnecter( QObject * parent = nullptr );

Q_SIGNALS:
  void edgeCreated(PortItem*, PortItem*);
};

class FlowSceneGraph : public QObject
{
  Q_OBJECT

public:
  FlowSceneGraph( QObject * parent = nullptr );
};

class FlowScene : public QGraphicsScene
{
  Q_OBJECT

public:
  FlowScene( QObject * parent = nullptr );

private:
  void setupActions();

private slots:
  void slotLoad();
  void slotSave();
  void slotSaveAs();
  void run();

private:
  boost::optional<QString> filename;
  bool modified = false;

  FlowSceneGraph * graph;
  FlowScenePortConnecter * connecter;

  QAction * loadAction, * saveAction, * saveAsAction, * runAction;
};

// Local Variables:
// c-basic-offset: 2
// End:
