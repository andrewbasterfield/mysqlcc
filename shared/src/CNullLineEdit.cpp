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
#include "CNullLineEdit.h"
#include <qevent.h>

CNullLineEdit::CNullLineEdit(QWidget * parent, const char * name)
: QLineEdit(parent, name), is_null(false)
{
  if (!name)
    setName("CNullLineEdit");
  last_key = 0;
  v = new CNullLineEditValidator(this);
}

CNullLineEdit::~CNullLineEdit()
{
  if (v != 0)
  {
    delete v;
    v = 0;
  }
}

void CNullLineEdit::setNull(bool b)
{
  is_null = b;
  if (b && text() != "[NULL]")
    setText("[NULL]");
}

void CNullLineEdit::keyPressEvent(QKeyEvent * e)
{
  if (last_key == Qt::Key_Control && e->key() == Qt::Key_0 && !isNull())
  {
    setNull(true);
    last_key = 0;    
    e->ignore();
    return;
  }
  else
  {
    if (e->key() != Qt::Key_Control && isNull())
    {
      setText(QString::null);
      setNull(false);
    }
    last_key = e->key();
    QLineEdit::keyPressEvent(e);
  }  
}

