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

#include "circuit_view.hpp"

#include <memory>

#include <QtGui/QClipboard>
#include <QtGui/QIcon>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QMenu>

#include <boost/foreach.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/irange.hpp>

#include <reversible/target_tags.hpp>
#include <reversible/functions/find_lines.hpp>
#include <reversible/io/create_image.hpp>

using namespace cirkit;

class CircuitLineItem : public QGraphicsItemGroup
{
public:
  CircuitLineItem( unsigned index, unsigned width, QGraphicsItem * parent = nullptr )
    : QGraphicsItemGroup( parent )
  {
    setToolTip( boost::str( boost::format( "<b><font color=\"#606060\">Line:</font></b> %d" ) % index ).c_str() );

    unsigned x = 0u;
    for ( unsigned i : boost::irange( 0u, width + 1u ) )
    {
      unsigned ewidth = ( i == 0u || i == width ) ? 15u : 30u;
      addToGroup( new QGraphicsLineItem( x, index * 30u, x + ewidth, index * 30u, this ) );
      x += ewidth;
    }
  }
};

class GateItem : public QGraphicsItemGroup
{
public:
  GateItem( const gate& g, unsigned index, const circuit& circ, QGraphicsItem * parent = nullptr )
    : QGraphicsItemGroup( parent )
  {
    std::vector<unsigned> lines;
    find_non_empty_lines( g, std::back_inserter( lines ) );
    boost::sort( lines );

    if ( lines.size() > 1u )
    {
      auto circuitLine = new QGraphicsLineItem( 0, lines.front() * 30, 0, lines.back() * 30, this );
      addToGroup( circuitLine );
    }

    for ( int t : g.targets() )
    {
      if ( is_toffoli( g ) )
      {
        auto target = new QGraphicsEllipseItem( -10, t * 30 - 10, 20, 20, this );
        auto targetLine = new QGraphicsLineItem( 0, t * 30 - 10, 0, t * 30 + 10, this );
        auto targetLine2 = new QGraphicsLineItem( -10, t * 30, 10, t * 30, this );
        addToGroup( target );
        addToGroup( targetLine );
        addToGroup( targetLine2 );
      }
      else
      {
        assert( false );
      }
    }

    for ( const auto& v : g.controls() )
    {
      auto control = new QGraphicsEllipseItem( -5, (int)v.line() * 30 - 5, 10, 10, this );
      control->setBrush( v.polarity() ? Qt::black : Qt::white );
      addToGroup( control );
    }
  }
};

class CircuitView::Private
{
public:
  Private() {}

  QAction * mLatexToClipboardAction;

  std::shared_ptr<circuit> mCircuit;
};

CircuitView::CircuitView( QWidget * parent )
  : QGraphicsView( parent ),
    d( new Private() )
{
  setScene( new QGraphicsScene( this ) );
  scene()->setBackgroundBrush( Qt::white );

  setupActions();
  setupContextMenu();
}

void CircuitView::load( const std::shared_ptr<circuit>& circ )
{
  d->mCircuit = circ;

  boost::for_each( scene()->items(), [this]( QGraphicsItem* item ) { scene()->removeItem( item ); } );

  unsigned width = 30u * circ->num_gates();

  for ( unsigned i : boost::irange( 0u, circ->lines() ) )
  {
    auto line = new CircuitLineItem( i, circ->num_gates() );
    scene()->addItem( line );

    addLineLabel( 0u, i * 30u, circ->inputs()[i].c_str(), Qt::AlignRight, (bool)circ->constants()[i] );
    addLineLabel( width, i * 30u, circ->outputs()[i].c_str(), Qt::AlignLeft, circ->garbage()[i] );
  }

  unsigned index = 0u;
  for ( const gate& g : *circ )
  {
    auto gateItem = new GateItem( g, index, *circ );
    gateItem->setPos( index * 30u + 15u, 0u );
    scene()->addItem( gateItem );
    ++index;
  }
}

void CircuitView::saveImage( const QString& filename ) const
{
  QPixmap pixmap( scene()->width(), scene()->height() );
  QPainter painter( &pixmap );
  scene()->render( &painter );
  pixmap.save( filename );
  painter.end();
}

void CircuitView::setupActions()
{
  d->mLatexToClipboardAction = new QAction( QIcon::fromTheme( "text-x-tex" ), "&LaTeX to Clipboard", this );
  connect( d->mLatexToClipboardAction, SIGNAL( triggered() ), SLOT( copyLatexToClipboard() ) );
}

void CircuitView::setupContextMenu()
{
  setContextMenuPolicy( Qt::CustomContextMenu );
  connect( this, SIGNAL( customContextMenuRequested(const QPoint&) ), SLOT( showContextMenu(const QPoint&) ) );
}

void CircuitView::showContextMenu( const QPoint& pos )
{
  QMenu * menu = new QMenu( this );
  menu->addAction( d->mLatexToClipboardAction );
  menu->popup( mapToGlobal( pos ) );
}

void CircuitView::copyLatexToClipboard()
{
  if ( d->mCircuit )
  {
    std::stringstream sstream;
    create_tikz_settings settings;
    create_image( sstream, *d->mCircuit, settings );
    QApplication::clipboard()->setText( QString::fromStdString( sstream.str() ) );
  }
}

QGraphicsTextItem * CircuitView::addLineLabel( int x, int y, QString text, unsigned align, bool color )
{
  auto textItem = scene()->addText( text );
  textItem->setPlainText( text );

  if ( align == Qt::AlignRight )
  {
    x -= textItem->boundingRect().width();
  }

  textItem->setPos( x, y - 12 );

  if ( color )
  {
    textItem->setDefaultTextColor( Qt::red );
  }

  return textItem;
}

#include "src/circuit_view.moc"

// Local Variables:
// c-basic-offset: 2
// End:
