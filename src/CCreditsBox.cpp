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
#include "CCreditsBox.h"
#include <qlayout.h>
#include <qlabel.h>
#include <qwhatsthis.h>

/*
This class shows the Credits Window in the "Help" menu item.
*/
CCreditsWidget::CCreditsWidget( QWidget* parent,  const char* name, WFlags fl )
: QWidget( parent, name, fl )
{
#ifdef DEBUG
  qDebug("CCreditsWidget::CCreditsWidget()");
#endif
  
  if (!name)
    setName("CCreditsWidget");
  creditsWidgetLayout = new QGridLayout( this, 1, 1, 1, 1, "creditsWidgetLayout"); 
  
  TextLabel3_2 = new QLabel( this, "TextLabel3_2" );
  TextLabel3_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)2, 0, 0, TextLabel3_2->sizePolicy().hasHeightForWidth() ) );
  TextLabel3_2->setFrameShape( QLabel::Panel );
  TextLabel3_2->setFrameShadow( QLabel::Sunken );
  TextLabel3_2->setText(tr(" Volker Hilsheimer<br>\n"
    "Trolltech Support Team<br>\n"
    "Tarmo Järvalt<br>\n"
    "Lenz Grimmer<br>\n"
    "Alexander Keremidarski<br>\n"
    "Matt Wagner<br>\n"
	  "Jeremy Cole<br>\n"
    "Tom Basil<br>\n"
    "And many others<br>") );
  TextLabel3_2->setAlignment( int( QLabel::AlignTop ) );
  QWhatsThis::add( TextLabel3_2,tr("People who contributed indirectly to this development." ) );
  
  creditsWidgetLayout->addWidget( TextLabel3_2, 4, 0 );
  
  TextLabel2_2 = new QLabel( this, "TextLabel2_2" );
  TextLabel2_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, TextLabel2_2->sizePolicy().hasHeightForWidth() ) );
  QFont TextLabel2_2_font(  TextLabel2_2->font() );
  TextLabel2_2_font.setBold(true);
  TextLabel2_2->setFont( TextLabel2_2_font ); 
  TextLabel2_2->setText(tr("Special Thanks To:" ) );
  
  creditsWidgetLayout->addWidget( TextLabel2_2, 3, 0 );
  
  TextLabel2 = new QLabel( this, "TextLabel2" );
  TextLabel2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, TextLabel2->sizePolicy().hasHeightForWidth() ) );
  QFont TextLabel2_font(  TextLabel2->font() );
  TextLabel2_font.setBold(true);
  TextLabel2->setFont( TextLabel2_font ); 
  TextLabel2->setText( "\n" +tr("Contributors:" ) );
  
  creditsWidgetLayout->addWidget( TextLabel2, 0, 0 );
  
  TextLabel3 = new QLabel( this, "TextLabel3" );
  TextLabel3->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, TextLabel3->sizePolicy().hasHeightForWidth() ) );
  TextLabel3->setFrameShape( QLabel::Panel );
  TextLabel3->setFrameShadow( QLabel::Sunken );
  TextLabel3->setText(tr(" <b>Jorge del Conde</b> [jorge@mysql.com]<br>\n"
    "<b>Sasha Pachev</b> [sasha@mysql.com]<br>\n"
  	"<b>Michael Widenius</b> [monty@mysql.com]<br>\n"
    "<b>Rafal Kurczyna</b> [rafal@kurczyna.com]<br>\n"
	  "<b>Reginald Stadlbauer</b> [reggie@trolltech.com]<br>\n"
	  "<b>Alexander M. Stetsenko</b> [ams@yandex.ru]<br>\n"
	  "<b>Adam Hooper</b> [adamh@densi.com]<br>\n"
    "<b>Peter O'Gorman</b> [peter@pogma.com]<br>\n"
    "<b>Alexander Ustimenko</b> [ustimenko@nsaem.ru]<br>\n"
	  "<b>Deng Jie</b> [deng@dengjie.com]<br>\n"
	  "<b>Mat Leinmueller</b> [m_leinmuller@hotmail.com]<br>\n"
	  "<b>Philippe Boxho</b> [boxhoph@ibelgique.com]<br>\n"
    "<b>Andrea Decorte</b> [klenje@adriacom.it]<br>\n"
    "<b>Radoslaw Zawartko</b> [radzaw@lnet.szn.pl]<br>\n"
    "<b>Adam Majer</b> [adamm@galacticasoftware.com]<br>\n"
    "<b>Kyung-uk Son</b> [hey_calm@yahoo.co.kr]"));
  
  TextLabel3->setAlignment( int( QLabel::AlignTop ) );
  QWhatsThis::add( TextLabel3,tr("People who contributed directly to this development." ) );
  
  creditsWidgetLayout->addWidget( TextLabel3, 1, 0 );
  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed );
  creditsWidgetLayout->addItem( spacer, 2, 0 );
}

CCreditsWidget::~CCreditsWidget()
{
#ifdef DEBUG
  qDebug("CCreditsWidget::~CCreditsWidget()");
#endif

}

CCreditsBox::CCreditsBox( QWidget* parent,  const char* name, bool modal, WFlags fl )
: CInfoDialog( parent, name, modal, fl )
{
#ifdef DEBUG
  qDebug("CCreditsBox::CCreditsBox()");
#endif

  if ( !name )
    setName( "CCreditsBox" );
  setCaption(tr("Credits"));
  creditsWidget = new CCreditsWidget(this, "creditsWidget");
  addMainWidget(creditsWidget);
}

CCreditsBox::~CCreditsBox()
{
#ifdef DEBUG
  qDebug("CCreditsBox::~CCreditsBox()");
#endif

}
