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

#include "viewer_window.hpp"

#include <fstream>
#include <memory>

#include <QtCore/QSize>
#include <QtGui/QIcon>
#include <QtWidgets/QAction>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>

#include <src/circuit_view.hpp>
#include <src/truth_table_widget.hpp>

#include <reversible/circuit.hpp>
#include <reversible/io/create_image.hpp>
#include <reversible/io/read_realization.hpp>

using namespace cirkit;

class ViewerWindow::Private
{
public:
  Private() {}

  QAction * mOpenAction, * mImageAction, * mLatexAction, * mExitAction;
  QAction * mInfoAction, * mSpecAction, * mPartialAction;
  QAction * mAboutAction;

  CircuitView * mCircuitView;

  std::shared_ptr<circuit> mCircuit;
};

ViewerWindow::ViewerWindow() : QMainWindow(), d( new Private() )
{
  setWindowTitle( "RevKit Viewer" );

  setupActions();
  setupMenuBar();
  setupToolBar();
  statusBar();

  d->mCircuitView = new CircuitView( this );
  setCentralWidget( d->mCircuitView );

  resize( 600, 300 );
}

CircuitView * ViewerWindow::circuitView() const
{
  return d->mCircuitView;
}

void ViewerWindow::open()
{
  QString filename = QFileDialog::getOpenFileName( this, "Open Circuit", "", "RevLib circuit (*.real)" );
  if ( !filename.isEmpty() )
  {
    openFromFilename( filename );
  }
}

void ViewerWindow::openFromFilename( const QString& filename )
{
  d->mCircuit.reset( new circuit );
  read_realization( *d->mCircuit, filename.toStdString() );
  d->mCircuitView->load( d->mCircuit );
}

void ViewerWindow::saveImage()
{
  QString filename = QFileDialog::getSaveFileName( this, "Save as Image", "", "PNG image (*.png);;JPG image (*.jpg)" );
  if ( !filename.isEmpty() )
  {
    d->mCircuitView->saveImage( filename );
  }
}

void ViewerWindow::saveLatex()
{
  QString filename = QFileDialog::getSaveFileName( this, "Save as LaTeX", "", "LaTeX file (*.tex)" );
  if ( !filename.isEmpty() )
  {
    std::filebuf fb;
    fb.open( filename.toLatin1(), std::ios::out );
    std::ostream os( &fb );
    create_tikz_settings settings;
    create_image( os, *d->mCircuit, settings );
    fb.close();
  }
}

void ViewerWindow::showTruthTable()
{
  QDialog * dialog = new QDialog( this, Qt::Dialog );
  dialog->setWindowTitle( "Truth Table" );

  TruthTableWidget * table = new TruthTableWidget( *d->mCircuit, dialog );

  dialog->setLayout( new QVBoxLayout() );
  dialog->layout()->setMargin( 0u );
  dialog->layout()->addWidget( table );

  dialog->exec();
}

void ViewerWindow::setupActions()
{
  d->mOpenAction = new QAction( QIcon::fromTheme( "document-open" ), "&Open...", this );
  d->mOpenAction->setStatusTip( "Opens a circuit realization in RevLib format" );

  d->mImageAction = new QAction( QIcon::fromTheme( "image-x-generic" ), "Save as &Image...", this );
  d->mImageAction->setStatusTip( "Saves the circuit as an image file (PNG or JPG)" );
  d->mLatexAction = new QAction( QIcon::fromTheme( "text-x-tex" ), "Save as &LaTeX...", this );
  d->mLatexAction->setStatusTip( "Saves the LaTeX code to generate this circuit" );
  d->mExitAction = new QAction( QIcon::fromTheme( "application-exit" ), "&Quit", this );
  d->mExitAction->setStatusTip( "Quits the program" );

  d->mInfoAction = new QAction( QIcon::fromTheme( "dialog-information" ), "&Circuit details", this );
  d->mInfoAction->setStatusTip( "Opens a dialog with circuit information" );
  d->mSpecAction = new QAction( QIcon::fromTheme( "view-form-table" ), "&View truth table", this );
  d->mSpecAction->setStatusTip( "Displays the full truth table of the circuit, obtained by simulation" );
  d->mPartialAction = new QAction( QIcon::fromTheme( "view-form-table" ), "&View partial truth table", this );
  d->mPartialAction->setStatusTip( "Displays a truth table only for non-constant and non-garbage signals" );

  d->mAboutAction = new QAction( QIcon::fromTheme( "help-about" ), "&About", this );
  d->mAboutAction->setStatusTip( "Displays information about the RevKit Viewer" );

  connect( d->mOpenAction, SIGNAL( triggered() ), SLOT( open() ) );
  connect( d->mImageAction, SIGNAL( triggered() ), SLOT( saveImage() ) );
  connect( d->mLatexAction, SIGNAL( triggered() ), SLOT( saveLatex() ) );
  connect( d->mExitAction, SIGNAL( triggered() ), SLOT( close() ) );

  connect( d->mSpecAction, SIGNAL( triggered() ), SLOT( showTruthTable() ) );
}

void ViewerWindow::setupMenuBar()
{
  QMenu * file = menuBar()->addMenu( "&File" );
  file->addAction( d->mOpenAction );
  file->addAction( d->mImageAction );
  file->addAction( d->mLatexAction );
  file->addSeparator();
  file->addAction( d->mExitAction );

  QMenu * view = menuBar()->addMenu( "&View" );
  view->addAction( d->mInfoAction );
  view->addSeparator();
  view->addAction( d->mSpecAction );
  view->addAction( d->mPartialAction );

  QMenu * help = menuBar()->addMenu( "&Help" );
  help->addAction( d->mAboutAction );
}

void ViewerWindow::setupToolBar()
{
  QToolBar * toolbar = addToolBar( "Main" );
  toolbar->setIconSize( QSize( 32, 32 ) );

  toolbar->addAction( d->mOpenAction );
  toolbar->addAction( d->mImageAction );
  toolbar->addSeparator();
  toolbar->addAction( d->mInfoAction );
  toolbar->addAction( d->mPartialAction );
}

#include "programs/viewer/viewer_window.moc"

// Local Variables:
// c-basic-offset: 2
// End:
