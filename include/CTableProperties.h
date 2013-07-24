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
#ifndef CTABLEPROPERTIES_H
#define CTABLEPROPERTIES_H

#include "CMyWindow.h"
#include "CProperties.h"
//Added by qt3to4:
#include <Q3GridLayout>

class CMySQLServer;
class QPushButton;


class CTablePropertiesWidget : public CProperties
{ 
  Q_OBJECT
    
public:
  CTablePropertiesWidget(QWidget* parent,  CMySQLServer *m, const QString & t, const QString & d);
    
public slots:
  void refresh();
    
private:  
  void insertData();    
  QString tableName;
  QString dbname;
};



class CTableProperties : public CMyWindow
{
  Q_OBJECT
    
public:
  CTableProperties(QWidget* parent, CMySQLServer *m, const QString &tablename, const QString &dbname, const char* name = 0);
  
private:
  QPushButton* closeButton;
  Q3GridLayout* Form1Layout;
};

#endif // CTABLEPROPERTIES_H
