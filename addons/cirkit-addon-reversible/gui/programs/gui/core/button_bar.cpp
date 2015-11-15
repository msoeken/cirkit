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

#include "button_bar.hpp"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

ButtonBar::ButtonBar( QWidget * parent ) : QWidget( parent )
{
  QBoxLayout * layout;

  buttons = new QWidget( this );
  buttons->setLayout( layout = new QHBoxLayout() );
  layout->setMargin( 0u );
  layout->setSpacing( 0u );
  layout->addStretch();

  stack = new QStackedWidget( this );

  setLayout( layout = new QVBoxLayout() );
  layout->setMargin( 0u );
  layout->setSpacing( 0u );
  layout->addWidget( stack );
  layout->addWidget( buttons );
}

void ButtonBar::addWidget( QWidget * page, const QIcon& icon, const QString& text )
{
  auto * button = new QPushButton( icon, text, buttons );
  button->setCheckable( true );
  button->setFlat( true );
  button->installEventFilter( this );

  if ( buttons->children().size() == 2 )
  {
    button->setChecked( true );
  }
  static_cast<QBoxLayout*>( buttons->layout() )->insertWidget( buttons->children().size() - 2, button );
  stack->addWidget( page );
}

void ButtonBar::setCurrentIndex( int index )
{
  if ( index == stack->currentIndex() ) return;

  eventFilter( buttons->children()[1 + index], new QEvent( QEvent::MouseButtonPress ) );
  stack->setCurrentIndex( index );
}

int ButtonBar::currentIndex() const
{
  return stack->currentIndex();
}


bool ButtonBar::eventFilter( QObject * obj, QEvent * event )
{
  auto * button = dynamic_cast<QPushButton*>( obj );

  if ( button && event->type() == QEvent::MouseButtonPress )
  {
    int index = buttons->children().indexOf( obj ) - 1;
    for ( auto * c : buttons->children() )
    {
      auto * cbutton = dynamic_cast<QPushButton*>( c );
      if ( cbutton )
      {
        cbutton->setChecked( false );
      }
    }

    if ( currentIndex() == index )
    {
      stack->setVisible( !stack->isVisible() );
      button->setChecked( stack->isVisible() );
    }
    else
    {
      stack->setVisible( true );
      button->setChecked( true );
    }
    stack->setCurrentIndex( index );
    return true;
  }

  return QWidget::eventFilter( obj, event );
}

#include "programs/gui/core/button_bar.moc"

// Local Variables:
// c-basic-offset: 2
// End:
