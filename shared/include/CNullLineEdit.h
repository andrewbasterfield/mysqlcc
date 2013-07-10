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
#ifndef CNULLLINEEDIT_H
#define CNULLLINEEDIT_H

#include <qlineedit.h>
#include <qvalidator.h>

class QKeyEvent;
class CNullLineEditValidator;


class CNullLineEdit : public QLineEdit
{
  Q_OBJECT
public:
  CNullLineEdit(QWidget * parent, const char * name = 0);
  ~CNullLineEdit();
  bool isNull() const { return is_null; }
  void setNull(bool b);    

private:
  CNullLineEditValidator* v;
  void keyPressEvent(QKeyEvent * e);
  bool is_null;
  int last_key;
};

class CNullLineEditValidator : public QValidator
{
  Q_OBJECT

public:
  CNullLineEditValidator(CNullLineEdit* parent, const char* name=0) 
  : QValidator(parent, name)
  {
      parent_editor = parent;
  }

  State validate(QString& input, int &) const
  {
    if (int(qstrlen(input.local8Bit())) <= parent_editor->maxLength())    
      return Acceptable;
    else
      return Invalid;
  }

private:
  CNullLineEdit* parent_editor;
};

#endif
