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
#ifndef CABOUTBOX_H
#define CABOUTBOX_H

#include <qtextedit.h>
#include "CInfoDialog.h"

class CAboutBox : public CInfoDialog
{ 
  Q_OBJECT
    
public:
  CAboutBox( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
  
private:
  QTextEdit* aboutWidget;
};

#endif // CABOUTBOX_H
