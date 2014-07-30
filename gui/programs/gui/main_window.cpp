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

#include "main_window.hpp"

MainWindow::MainWindow( QWidget * parent ) : QMainWindow( parent )
{
  setupWidgets();
  setupDockWidgets();
  setupTools();
  setupActions();
  setupToolBars();

  widget->ui.tabWidget->slotNew();
}

void MainWindow::setupWidgets()
{
  widget = new MainWidget( this );
  setCentralWidget( widget );
}

void MainWindow::setupDockWidgets()
{
  messagesWidget = new MessagesWidget( this );
  runWidget = new QProgressBar( this );

  widget->ui.buttonBar->addWidget( messagesWidget, QIcon::fromTheme( "utilities-system-monitor" ), "Messages" );
  widget->ui.buttonBar->addWidget( runWidget, QIcon::fromTheme( "run-build" ), "Run" );

  connect( messagesWidget, SIGNAL( messageAdded() ), this, SLOT( messageAdded() ) );
}

void MainWindow::setupTools()
{
  widget->addToolCategory( "Sources" );
  widget->addToolCategory( "Sinks" );
  widget->addToolCategory( "Synthesis" );
  widget->addToolCategory( "Optimization" );
  widget->addToolCategory( "Verification" );
  widget->addToolCategory( "Utilities" );

  widget->ui.toolBox->expandAll();
}

void MainWindow::setupActions()
{
  settingsAction = new QAction( QIcon::fromTheme( "configure" ), "Settings", this );
  connect( settingsAction, SIGNAL( triggered() ), this, SLOT( configure() ) );
}

void MainWindow::setupToolBars()
{
  mainBar = addToolBar( "main" );
  mainBar->setIconSize( QSize( 22, 22 ) );
  mainBar->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
  mainBar->addAction( widget->ui.tabWidget->newAction );
  mainBar->addAction( widget->ui.tabWidget->openAction );
  mainBar->addAction( widget->ui.tabWidget->saveAction );
  mainBar->addAction( widget->ui.tabWidget->saveAsAction );
  mainBar->addSeparator();
  mainBar->addAction( settingsAction );
  mainBar->addSeparator();
  mainBar->addAction( widget->ui.tabWidget->runAction );
}

void MainWindow::messageAdded()
{
  widget->ui.buttonBar->setCurrentIndex( 0 );
}

void MainWindow::configure()
{
  // TODO
}

#include "programs/gui/main_window.moc"

// Local Variables:
// c-basic-offset: 2
// End:

