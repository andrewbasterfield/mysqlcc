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
#include "CFieldEditorWidget.h"
#include <qmainwindow.h>

CFieldEditorWidget::CFieldEditorWidget(QWidget *parent, bool readonly, const char* name)
:QWidget(parent, name, 0)
{  
  setReadOnly(readonly);
}

void CFieldEditorWidget::setReadOnly(bool b)
{
  is_read_only = b;
  emit set_read_only(is_read_only);
}

void CFieldEditorWidget::setCloseOnApply(bool b)
{
  close_on_apply = b;
  emit set_close_on_apply(close_on_apply);
}

void CFieldEditorWidget::message(ushort type, const QString &message)
{
  emit send_message(type, message);
}

void CFieldEditorWidget::addHotKeyEditorMenu()
{
  emit add_hot_key_editor_menu();
}
