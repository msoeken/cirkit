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

#include "main_widget.hpp"

#include <QtWidgets/QWhatsThis>

MainWidget::MainWidget( QWidget * parent ) : QWidget( parent )
{
  ui.setupUi( this );
  ui.helpItems->setText( "What's This?" );
  ui.helpItems->setIcon( QIcon::fromTheme( "help-contextual" ) );
  connect( ui.helpItems, SIGNAL( released() ), this, SLOT( help() ) );

  ui.splitter->setSizes( { 200, 1000 } );
  ui.splitter_2->setSizes( { 1000, 200 } );

  connect( ui.filterLine, SIGNAL( textChanged(const QString&) ), ui.toolBox, SLOT( setFilter(const QString&) ) );

  ui.filterLine->setPlaceholderText( "Filter Items" );
}

void MainWidget::addToolCategory( const QString& name )
{
  ui.toolBox->addCategory( name );
}

void MainWidget::help()
{
  QWhatsThis::enterWhatsThisMode();
}

#include "programs/gui/core/main_widget.moc"

// Local Variables:
// c-basic-offset: 2
// End:
