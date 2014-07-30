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

#include <QtCore/QEvent>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QWidget>

class ButtonBar : public QWidget
{
  Q_OBJECT

public:
  ButtonBar( QWidget * parent = nullptr );

  void addWidget( QWidget * page, const QIcon& icon, const QString& text );

  void setCurrentIndex( int index );
  int currentIndex() const;

protected:
  bool eventFilter( QObject * obj, QEvent * event );

private:
  QWidget * buttons;
  QStackedWidget * stack;
};

// Local Variables:
// c-basic-offset: 2
// End:
