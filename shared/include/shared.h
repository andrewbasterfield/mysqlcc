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
#ifndef SHARED_H
#define SHARED_H

#ifdef WIN32
#include <windows.h>
#else
#include <stdlib.h>
#endif

#include <stddef.h>
#include <qstring.h>
#include <qsound.h>
#include <qapplication.h>
#include <q3filedialog.h>
#include <qfile.h>

#if !defined(max)
#define max(a, b)       ((a) > (b) ? (a) : (b))
#define min(a, b)       ((a) < (b) ? (a) : (b))
#endif

#ifndef HAS_NICE_TIME
#define HAS_NICE_TIME
#endif

#define MESSAGE 1100
#define INFORMATION 1101
#define WARNING 1102
#define CRITICAL 1103
#define SQL_DEBUG 1104
#define HISTORY 1105
#define DEFAULT 1106
#define FINISHED 1107
#define NEW_QUERY_TAB 1108
#define NEW_QUERY_WINDOW 1109
#define SET_ACTIVE_PANEL 1110
#define PROCESS_EVENTS 1111

extern QString charReplace(const QString &str, const QChar &from, const QString &to);
extern const QString booltostr(bool b);
extern const QString booltoyesno(bool b);
extern bool strtobool(const QString & b);
extern const QString nice_time(ulong sec);
extern const char * debug_string(const QString &);
extern void playSound(const QString &fileName);
extern QString getSaveFileName(const QString &fileName, const QString &default_ext, const QString &ext_desc);
extern QString capitalize(const QString &s);
extern QString getFileName(const QString &fileWithPath);
extern bool validateAlphaNum(const QString & str);
#ifndef strmov
extern char *strmov(register char *dst, register const char *src);
#endif

#endif
