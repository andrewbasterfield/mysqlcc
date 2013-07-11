/* Copyright (C) 2003 MySQL AB & Jorge del Conde

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.
    
  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
  MA 02111-1307, USA 
*/
#if defined(__BCPLUSPLUS__)
#pragma warn -8080
#endif

#include "CInfoDialog.h"
#include <stddef.h>  
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include "globals.h"
#include "config.h"
#include "icons.h"

/*
This is the base class for dialogs such as CAboutBox & CCreditsBox.  This class takes care of
showing the left-side pixmap & the close buttons.
*/
CInfoDialog::CInfoDialog( QWidget* parent,  const char* name, bool modal, WFlags fl )
: QDialog( parent, name, modal, fl )
{
#ifdef DEBUG
  qDebug("CInfoDialog::CInfoDialog()");
#endif
  
  QPixmap image1( ( const char** ) infoDialogPicture_data );
  if ( !name )
    setName( "CInfoDialog" );
  resize( 472, 202 ); 
  setCaption(APPLICATION);
  setIcon(getPixmapIcon("applicationIcon"));
  CInfoDialogLayout = new QGridLayout( this, 1, 1, 4, 2, "CInfoDialogLayout"); 
  
  Copyright = new QLabel( this, "Copyright" );
  Copyright->setText(tr(COPYRIGHT));
  
  CInfoDialogLayout->addMultiCellWidget( Copyright, 1, 1, 1, 2 );
  
  Title = new QLabel( this, "Title" );
  QFont Title_font(  Title->font() );
  Title_font.setBold( TRUE );
  Title->setFont( Title_font );
  Title->setText("<b>" +  QString(APPLICATION) + " " +  QString(VERSION) + QString(BRANCH) + "</b>");  
  
  CInfoDialogLayout->addMultiCellWidget( Title, 0, 0, 1, 2 );
  
  PixmapLabel1 = new QLabel( this, "PixmapLabel1" );
  PixmapLabel1->setMinimumSize( QSize( 78, 0 ) );
  PixmapLabel1->setMaximumSize( QSize( 78, 32767 ) );
  QPalette pal;
  QColorGroup cg;
  cg.setColor( QColorGroup::Foreground, black );
  cg.setColor( QColorGroup::Button, QColor( 166, 202, 240) );
  cg.setColor( QColorGroup::Light, white );
  cg.setColor( QColorGroup::Midlight, QColor( 210, 228, 247) );
  cg.setColor( QColorGroup::Dark, QColor( 83, 101, 120) );
  cg.setColor( QColorGroup::Mid, QColor( 110, 134, 160) );
  cg.setColor( QColorGroup::Text, black );
  cg.setColor( QColorGroup::BrightText, white );
  cg.setColor( QColorGroup::ButtonText, black );
  cg.setColor( QColorGroup::Base, white );
  cg.setColor( QColorGroup::Background, QColor( 49, 156, 198) );
  cg.setColor( QColorGroup::Shadow, black );
  cg.setColor( QColorGroup::Highlight, QColor( 0, 0, 128) );
  cg.setColor( QColorGroup::HighlightedText, white );
  cg.setColor( QColorGroup::Link, black );
  cg.setColor( QColorGroup::LinkVisited, black );
  pal.setActive( cg );
  cg.setColor( QColorGroup::Foreground, black );
  cg.setColor( QColorGroup::Button, QColor( 166, 202, 240) );
  cg.setColor( QColorGroup::Light, white );
  cg.setColor( QColorGroup::Midlight, QColor( 197, 225, 255) );
  cg.setColor( QColorGroup::Dark, QColor( 83, 101, 120) );
  cg.setColor( QColorGroup::Mid, QColor( 110, 134, 160) );
  cg.setColor( QColorGroup::Text, black );
  cg.setColor( QColorGroup::BrightText, white );
  cg.setColor( QColorGroup::ButtonText, black );
  cg.setColor( QColorGroup::Base, white );
  cg.setColor( QColorGroup::Background, QColor( 49, 156, 198) );
  cg.setColor( QColorGroup::Shadow, black );
  cg.setColor( QColorGroup::Highlight, QColor( 0, 0, 128) );
  cg.setColor( QColorGroup::HighlightedText, white );
  cg.setColor( QColorGroup::Link, QColor( 0, 0, 255) );
  cg.setColor( QColorGroup::LinkVisited, QColor( 255, 0, 255) );
  pal.setInactive( cg );
  cg.setColor( QColorGroup::Foreground, QColor( 128, 128, 128) );
  cg.setColor( QColorGroup::Button, QColor( 166, 202, 240) );
  cg.setColor( QColorGroup::Light, white );
  cg.setColor( QColorGroup::Midlight, QColor( 197, 225, 255) );
  cg.setColor( QColorGroup::Dark, QColor( 83, 101, 120) );
  cg.setColor( QColorGroup::Mid, QColor( 110, 134, 160) );
  cg.setColor( QColorGroup::Text, QColor( 128, 128, 128) );
  cg.setColor( QColorGroup::BrightText, white );
  cg.setColor( QColorGroup::ButtonText, QColor( 128, 128, 128) );
  cg.setColor( QColorGroup::Base, white );
  cg.setColor( QColorGroup::Background, QColor( 49, 156, 198) );
  cg.setColor( QColorGroup::Shadow, black );
  cg.setColor( QColorGroup::Highlight, QColor( 0, 0, 128) );
  cg.setColor( QColorGroup::HighlightedText, white );
  cg.setColor( QColorGroup::Link, QColor( 0, 0, 255) );
  cg.setColor( QColorGroup::LinkVisited, QColor( 255, 0, 255) );
  pal.setDisabled( cg );
  PixmapLabel1->setPalette( pal );
  PixmapLabel1->setFrameShape( QLabel::Box );
  PixmapLabel1->setFrameShadow( QLabel::Plain );
  PixmapLabel1->setPixmap( image1 );
  PixmapLabel1->setScaledContents( FALSE );
  PixmapLabel1->setAlignment( int( QLabel::AlignBottom ) );
  
  CInfoDialogLayout->addMultiCellWidget( PixmapLabel1, 0, 2, 0, 0 );
  
  m_pClosePushButton = new QPushButton( this, "m_pClosePushButton" );
  m_pClosePushButton->setMinimumSize( QSize( 70, 0 ) );
  m_pClosePushButton->setMaximumSize( QSize( 70, 32767 ) );
  m_pClosePushButton->setText(tr("&Close" ) );
  QWhatsThis::add( m_pClosePushButton,tr("Click to close this dialog." ) );
  
  CInfoDialogLayout->addWidget( m_pClosePushButton, 3, 2 );
  
  TextLabel4 = new QLabel( this, "TextLabel4" );
  TextLabel4->setText(tr("For more information please visit http://www.mysql.com" ) );
  
  CInfoDialogLayout->addMultiCellWidget( TextLabel4, 3, 3, 0, 1 );
  connect(m_pClosePushButton, SIGNAL(clicked()), this, SLOT(close()));
}

/*
This function adds [widget] to the right side of the InfoDialog ...
*/
void CInfoDialog::addMainWidget(QWidget *widget)
{
#ifdef DEBUG
  qDebug("CInfoDialog::addMainWidget()");
#endif
  
  CInfoDialogLayout->addMultiCellWidget(widget, 2, 2, 1, 2 );
}

CInfoDialog::~CInfoDialog()
{
#ifdef DEBUG
  qDebug("CInfoDialog::~CInfoDialog()");
#endif

}

void CInfoDialog::setCaption(const QString &s)
{
  if (QDialog::caption() == s)
    return;

  if (myApp()->isMDI())
    QDialog::setCaption(s);
  else
  {
    QString app_name(SHORT_NAME);
    app_name += " - ";
    if (s.startsWith(app_name))
      QDialog::setCaption(s);
    else
      QDialog::setCaption(app_name + s);
  }
}
