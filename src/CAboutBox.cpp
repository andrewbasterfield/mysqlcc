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
#include "CAboutBox.h"
#include "config.h"

/*
This class shows the About Box in the "Help" menu item
*/
CAboutBox::CAboutBox( QWidget* parent,  const char* name, bool modal, Qt::WFlags fl )
: CInfoDialog( parent, name, modal, fl )
{
#ifdef DEBUG
  qDebug("CAboutBox::CAboutBox()");
#endif

  if ( !name )
    setName( "CAboutBox" );
  setCaption(tr("About") + " " + APPLICATION);
  aboutWidget = new Q3TextEdit( this, "infoWidget" );
  aboutWidget->setText(tr( "This program is free software; you can redistribute it and/or modify "
    "it under the terms of the GNU General Public License as published by "
    "the Free Software Foundation; either version 2 of the License, or "
    "(at your option) any later version.\n"
    "\n"
    "This program is distributed in the hope that it will be useful, "
    "but WITHOUT ANY WARRANTY; without even the implied warranty of "
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
    "GNU General Public License for more details.\n"
    "\n"
    "You should have received a copy of the GNU General Public License "
    "along with this program; if not, write to the Free Software "
    "Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA" ) );
  aboutWidget->setReadOnly(true);
  addMainWidget(aboutWidget);  
}
