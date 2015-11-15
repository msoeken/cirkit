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

#include <QtCore/QSortFilterProxyModel>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QTreeView>

class FlowItemSortFilterProxyModel;

class FlowItemTreeWidget : public QTreeView
{
  Q_OBJECT

public:
  FlowItemTreeWidget( QWidget * parent = nullptr );

public Q_SLOTS:
  void setFilter( const QString& pattern );

public:
  // TODO add second parameter
  void addCategory( const QString& category );

private:
  QStandardItemModel * _model;
  FlowItemSortFilterProxyModel * proxy;
};

class FlowItemSortFilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  FlowItemSortFilterProxyModel( QObject * parent = nullptr );

protected:
  bool filterAcceptsRow( int source_row, const QModelIndex& source_parent ) const;
};

// Local Variables:
// c-basic-offset: 2
// End:
