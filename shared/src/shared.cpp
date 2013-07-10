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
#include "shared.h"

/*
used for debugging string arguments.
*/
const char * debug_string(const QString &s)
{
  if (s.isNull())
    return "[NULL STRING]";
  else
    if (s.isEmpty())
      return "[EMPTY STRING]";
    else
      return (const char *) s;
}

/*
  This function replaces all occurances of character "from" in the string "str"
  with the string "to"
*/
QString charReplace(const QString &str, const QChar &from, const QString &to)
{
  QString tmpstr = QString::null;
  for (uint i=0; i < str.length(); i++)
  {
    QChar tmp = str.at(i);
    tmpstr += ((tmp == from) ? to : QString(tmp));
  }
  return tmpstr;
}

/*
This function returns a human-readable time from a number of seconds
*/
const QString nice_time(ulong sec)
{
  QString buff;  
  ulong tmp;
  if (sec >= 3600L*24)
  {
    tmp=sec/(3600L*24);
    sec-=3600L*24*tmp;
    buff += QString::number(tmp);
    buff += (tmp > 1) ? " " + qApp->translate("nice_time", "days") + " " : " " + qApp->translate("nice_time", "day") + " ";
  }
  if (sec >= 3600L)
  {
    tmp=sec/3600L;
    sec-=3600L*tmp;
    buff += QString::number(tmp);
    buff += (tmp > 1) ? " " + qApp->translate("nice_time", "hours") + " " : " " + qApp->translate("nice_time", "hour") + " ";    
  }
  if (sec >= 60)
  {
    tmp=sec/60;
    sec-=60*tmp;
    buff += QString::number(tmp);
    buff += " " + qApp->translate("nice_time", "min") + " ";
  }
  buff += QString::number(sec);
  buff += " " + qApp->translate("nice_time", "sec") + " ";
  return buff;  
}


/*
This function returns the strings "true" or "false" depending on the value of the
bool parameter "b"
*/
const QString booltostr(bool b)
{
  return b ? "true" : "false";
}


/*
This function returns the strings "Yes" or "No" depending on the value of the
bool parameter "b";  true == "Yes", false == "No".

NOTE:  The return values depend on the current translation file being used.
*/
const QString booltoyesno(bool b)
{ 
  return b ? qApp->translate("booltoyesno", "Yes") : qApp->translate("booltoyesno", "No");
}


/*
This function returns "true" if the parameter "b" is "true" ... it returns false otherwise.
*/
bool strtobool(const QString & b)
{ 
  return ((b.lower() == "true") ? true : false);
}


/*
playSound() is a wrapper for QSound::play()
*/
void playSound(const QString &fileName)
{ 
  if (!QSound::isAvailable() || fileName.isEmpty())
    return;
  if (!QFile::exists(fileName))
    return;
  QSound::play(fileName);
}


/*
getSaveFileName() pops up a file save dialog
*/
QString getSaveFileName(const QString &fileName, const QString &default_ext, const QString &ext_desc)
{ 
  QString fn = QFileDialog::getSaveFileName(fileName, ext_desc, 0);
  if (!fn.isEmpty())
  {
    if (!default_ext.isNull())
    {
      if (!fn.contains("." + default_ext, false))
        fn += "." + default_ext;
    }
    return fn;
  }
  return QString::null;
}


/*
capitalize() returns a string where the first letter is a capital letter and everything else is lower case.
*/
QString capitalize(const QString &s)
{ 
  if (s.isEmpty())
    return s;
  QString t = s.lower();
  t[0] = t[0].upper();
  return t;
}


/*
Strips the path out of [fileWithPath]
*/
QString getFileName(const QString &fileWithPath)
{
  QString tmp = charReplace(fileWithPath, '\\', "/");
  int pos = tmp.findRev('/');
  if (pos == -1)
    return tmp;
  return tmp.mid(pos + 1);
}

/*
Returns true if none of the characters in [str] does not match isLetterOrNumber() and validChars.
*/
bool validateAlphaNum(const QString & str)
{
  const QString validChars = "-_.+@:;,[](){}~#$! ";
  for (uint i=0; i < str.length() ; i++)
  {
    QChar tmp = str.at(i);
    if ((!tmp.isLetterOrNumber()) && (validChars.find(tmp) == -1))
      return false;
  }
  return true;
}

#ifndef strmov
char *strmov(register char *dst, register const char *src)
{ 
  while ((*dst++ = *src++)) ;
  return dst-1;
}
#endif

