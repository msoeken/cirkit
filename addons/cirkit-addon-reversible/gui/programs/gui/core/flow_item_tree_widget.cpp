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

#include "flow_item_tree_widget.hpp"

#include <QtWidgets/QHeaderView>

FlowItemTreeWidget::FlowItemTreeWidget( QWidget * parent ) : QTreeView( parent )
{
  setAnimated( true );
  setDragEnabled( true );
  setRootIsDecorated( false );
  setFrameShape( QFrame::NoFrame );
  setIconSize( QSize( 22, 22 ) );
  header()->setStretchLastSection( true );

  auto palette = viewport()->palette();
  palette.setColor( viewport()->backgroundRole(), Qt::transparent );
  palette.setColor( viewport()->foregroundRole(), palette.color( QPalette::WindowText ) );
  viewport()->setPalette( palette );

  _model = new QStandardItemModel();
  proxy = new FlowItemSortFilterProxyModel( this );
  proxy->setSourceModel( _model );
  setModel( proxy );

  header()->hide();
}

void FlowItemTreeWidget::addCategory( const QString& category )
{
  auto * root = new QStandardItem( category );
  root->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  _model->appendRow( root );

  root->setFlags( Qt::ItemIsEnabled );
  QFont f = root->font();
  f.setPointSize( 10 );
  f.setBold( true );
  root->setFont( f );

  // TODO
}

void FlowItemTreeWidget::setFilter( const QString& pattern )
{
  // TOOD
}

FlowItemSortFilterProxyModel::FlowItemSortFilterProxyModel( QObject * parent )
  : QSortFilterProxyModel( parent )
{
}

bool FlowItemSortFilterProxyModel::filterAcceptsRow( int source_row, const QModelIndex& source_parent ) const
{
  if ( !source_parent.isValid() ) return true;

  auto index = sourceModel()->index( source_row, 0, source_parent );
  return sourceModel()->data( index ).toString().contains( filterRegExp() );
}

// Local Variables:
// c-basic-offset: 2
// End:
