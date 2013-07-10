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
#ifndef CONFIG_H
#define CONFIG_H

#define DEBUG_LEVEL 3

#ifdef DEBUG_LEVEL
#define DEBUG
#endif

#undef DEBUG
#undef DEBUG_LEVEL

#ifndef NO_QT
#include "shared.h"
#endif

#define APPLICATION "MySQL Control Center"
#define EXENAME "mysqlcc"
#define VERSION "0.9.8"
#define MYSQLCC_VERSION 0x000904
#define BRANCH "-beta"
#define COPYRIGHT "Copyright (C) MySQL AB & Jorge del Conde 2003"
#define SHORT_NAME "MySQLCC"

#define CONNECTIONS_PATH "connections"
#define HISTORY_FILE_NAME "history"
#define HOTKEY_PATH "hotkeys"

#define MENU_NEW_WINDOW_FROM_HERE  100
#define MENU_CONNECT  101
#define MENU_DISCONNECT 102
#define MENU_QUERY  103
#define MENU_DELETE  104
#define MENU_NEW  105
#define MENU_REFRESH  106
#define MENU_EDIT 107
#define MENU_EXPORT 108
#define MENU_EMPTY  109
#define MENU_RENAME 110
#define MENU_IMPORT 111
#define MENU_SHOW 112
#define MENU_HIDE 113
#define MENU_NEW_WINDOW 114
#define MENU_SHOW_VARIABLES 115
#define MENU_SHOW_STATUS 116
#define MENU_SHOW_PROCESSLIST 117
#define MENU_PING 118
#define MENU_FLUSH 119
#define MENU_SHUTDOWN 120
#define MENU_KILL_PROCESS 121
#define MENU_PROPERTIES 122
#define MENU_SAVE 123
#define MENU_SAVE_ALL 124
#define MENU_SAVE_PROCESSLIST 125
#define MENU_SAVE_STATUS 126
#define MENU_SAVE_VARIABLES 127
#define MENU_COLUMNS_WINDOW 128
#define MENU_CLEAR_GRID 129
#define MENU_TRADITIONAL 130
#define MENU_OPEN 131
#define MENU_OPEN_AS 132
#define MENU_OPEN_TEXT 133
#define MENU_OPEN_IMAGE 134
#define MENU_OPEN_BINARY 135
#define MENU_LOAD 136
#define MENU_SHOW_TREE 137
#define MENU_HIDE_TREE 138
#define MENU_SHOW_MESSAGES 139
#define MENU_HIDE_MESSAGES 140
#define MENU_TOOLS 141
#define MENU_COPY 142
#define MENU_PASTE 143
#define MENU_CUT 144
#define MENU_SHOW_INNODB_STATUS 145
#define MENU_SAVE_INNODB_STATUS 146
#define MENU_SAVE_TO_FILE 147
#define MENU_INSERT 148

#define ANALYZE_TABLE 500
#define CHECK_TABLE 501
#define OPTIMIZE_TABLE 502
#define REPAIR_TABLE 503
#define SHOW_CREATE_TABLE 504

#define SQL_ALL_ROWS  1000
#define SQL_LIMIT  1001
#define SQL_QUERY  1002
#define SQL_PANEL  1003
#define RESULTS_PANEL  1004
#define ERROR_PANEL 1005
#define HISTORY_PANEL 1006

#define FLUSH_HOSTS 1500
#define FLUSH_LOGS 1501
#define FLUSH_PRIVILEGES 1502
#define FLUSH_TABLES 1503
#define FLUSH_TABLES_RL 1504
#define FLUSH_STATUS 1505
#define FLUSH_DES_KEY_FILE 1506
#define FLUSH_QUERY_CACHE 1507
#define FLUSH_USER_RESOURCES 1508

#define MAX_COLUMN_LENGTH 1024
#define NULL_TEXT "[NULL]"

#endif
