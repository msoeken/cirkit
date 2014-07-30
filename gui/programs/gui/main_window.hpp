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

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QToolBar>

#include <programs/gui/core/main_widget.hpp>
#include <programs/gui/core/messages_widget.hpp>

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow( QWidget * parent = nullptr );

private:
  void setupWidgets();
  void setupDockWidgets();
  void setupTools();
  void setupActions();
  void setupToolBars();

private Q_SLOTS:
  void messageAdded();
  void configure();

private:
  MainWidget * widget;

  MessagesWidget * messagesWidget;
  QProgressBar * runWidget;
  QAction * settingsAction;
  QToolBar * mainBar;
};

// Local Variables:
// c-basic-offset: 2
// End:
