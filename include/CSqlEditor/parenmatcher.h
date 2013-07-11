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
 /**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef PARENMATCHER_H
#define PARENMATCHER_H

#include <stddef.h>
#include <qstring.h>
#include <qvaluelist.h>

class QTextCursor;

struct  Paren
{
    Paren() : type( Open ), chr( ' ' ), pos( -1 ) {}
    Paren( int t, const QChar &c, int p ) : type( (Type)t ), chr( c ), pos( p ) {}
    enum Type { Open, Closed };
    Type type;
    QChar chr;
    int pos;

#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
    bool operator==( const Paren & ) const {
	return type == type && chr == chr && pos == pos; }
#endif
};

typedef QValueList<Paren> ParenList;

class  ParenMatcher
{
public:
    enum Selection {
	Match = 1,
	Mismatch
    };

    ParenMatcher();

    virtual bool match( QTextCursor *c );

    void setEnabled( bool b ) { enabled = b; }

private:
    bool checkOpenParen( QTextCursor *c );
    bool checkClosedParen( QTextCursor *c );

    bool enabled;

};

#endif
