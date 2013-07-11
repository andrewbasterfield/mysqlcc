TEMPLATE     = app
CONFIG       += qt thread
DEFINES      += QT_DLL
DEFINES      += HAVE_MYSQLCC_CONFIG
TARGET       = mysqlcc
LANGUAGE     = C++
MOC_DIR      = moc

INCLUDEPATH    += ./include \
                  ./shared/include \
                  ./include/CSqlEditor

TRANSLATIONS    = translations/Deutsch.ts \
                  translations/French.ts \
                  translations/Italian.ts \
                  translations/Polish.ts \
                  translations/Russian.ts \
                  translations/Simplified_Chinese.ts \
                  translations/Spanish.ts \
                  translations/Traditional_Chinese.ts

win32 {
      LIBS        += C:\mysql\lib\opt\libmysql.lib
      INCLUDEPATH += C:\mysql\include
      CONFIG      += windows
      RC_FILE      = win32/mysqlcc.rc
}

unix {
      LIBS        += /usr/lib/x86_64-linux-gnu/libmysqlclient.so  
      INCLUDEPATH += /usr/include/mysql
      CONFIG      += x11 
      OBJECTS_DIR  = tmp
}

macx {
        LIBS        += /usr/lib/x86_64-linux-gnu/libmysqlclient.so  -headerpad_max_install_names -lz -framework Carbon -framework CoreFoundation
        DEFINES     += QT_OSX_BUILD
        INCLUDEPATH += /usr/include/mysql
        OBJECTS_DIR  = tmp
        macxrmfiles.commands = $(DEL_FILE) mysqlcc.app/Contents/Info.plist
        macxcopyfiles.target = mysqlcc.app/Contents/Resources
        macxcopyfiles.commands = \
        $(MKDIR) mysqlcc.app/Contents/Resources && \
        $(COPY_FILE) macx/mysqlcc.icns mysqlcc.app/Contents/Resources/mysqlcc.icns && \
        $(COPY_FILE) macx/Info.plist mysqlcc.app/Contents/Info.plist && \
        $(COPY_FILE) *.wav mysqlcc.app/Contents/Resources/ && \
        $(MKDIR) mysqlcc.app/Contents/Resources/translations && \
        $(COPY_FILE) translations/*.qm mysqlcc.app/Contents/Resources/translations/ && \
        $(COPY_FILE) syntax.txt mysqlcc.app/Contents/Resources/syntax.txt
        macxcopyfiles.depends = macxrmfiles
        macxstandalone.depends = $(TARGET)
        macxstandalone.commands = \
        $(COPY_FILE) $(QTDIR)/lib/`otool -XD $(QTDIR)/lib/libqt-mt.dylib` mysqlcc.app/Contents/MacOS/Qt && \
        install_name_tool -change `otool -XD $(QTDIR)/lib/libqt-mt.dylib` \
        @executable_path/Qt mysqlcc.app/Contents/MacOS/mysqlcc
        QMAKE_EXTRA_UNIX_TARGETS += macxcopyfiles macxrmfiles macxstandalone
        POST_TARGETDEPS += mysqlcc.app/Contents/Resources
}

SOURCES     =  src/CAboutBox.cpp src/CAdministrationWindow.cpp src/CAppConfigDialog.cpp src/CApplication.cpp src/CConfigDialog.cpp src/CConsoleWindow.cpp src/CCreditsBox.cpp src/CDatabaseGroupItem.cpp src/CDatabaseItem.cpp src/CDatabaseListView.cpp src/CDatabaseListViewItem.cpp src/CDockWindow.cpp src/CFieldEditorWidget.cpp src/CFieldEditorWindow.cpp src/CHistoryView.cpp src/CHotKeyEditorDialog.cpp src/CImageViewer.cpp src/CInfoDialog.cpp src/CMDIWindow.cpp src/CMySQLServer.cpp src/CMyWindow.cpp src/CPlugin.cpp src/CPluginWindow.cpp src/CProperties.cpp src/CQueryTable.cpp src/CQueryWindow.cpp src/CQueryWindowOptionsDialog.cpp src/CRegisterServerDialog.cpp src/CServerAdministrationItem.cpp src/CServerItem.cpp src/CShowDatabaseGroupProperties.cpp src/CShowServerProperties.cpp src/CShowTableGroupProperties.cpp src/CShowTableProperties.cpp src/CSqlEditorFont.cpp src/CSqlTable.cpp src/CTable.cpp src/CTableFieldChooser.cpp src/CTableGroupItem.cpp src/CTableItem.cpp src/CTableProperties.cpp src/CTableSpecific.cpp src/CTableTools.cpp src/CTableWindow.cpp src/CTablesListBox.cpp src/CTextDialog.cpp src/CTextFieldEditor.cpp src/CToolBar.cpp src/CUserAdminItem.cpp src/CUserAdminWindow.cpp src/CUserItem.cpp src/globals.cpp src/main.cpp src/mysqlcc_main.cpp src/panels.cpp src/shared_menus.cpp src/CSqlEditor/CSqlEditor.cpp src/CSqlEditor/completion.cpp src/CSqlEditor/editor.cpp src/CSqlEditor/parenmatcher.cpp src/CSqlEditor/sqlcompletion.cpp src/CSqlEditor/syntaxhighliter_sql.cpp shared/src/CAction.cpp shared/src/CConfig.cpp shared/src/CMySQL.cpp shared/src/CMySQLQuery.cpp shared/src/CNullLineEdit.cpp shared/src/CSqlTableItem.cpp shared/src/shared.cpp

HEADERS     =  include/CAboutBox.h include/CAdministrationWindow.h include/CAppConfigDialog.h include/CApplication.h include/CConfigDialog.h include/CConsoleWindow.h include/CCreditsBox.h include/CDatabaseGroupItem.h include/CDatabaseItem.h include/CDatabaseListView.h include/CDatabaseListViewItem.h include/CDockWindow.h include/CFieldEditorWidget.h include/CFieldEditorWindow.h include/CHistoryView.h include/CHotKeyEditorDialog.h include/CImageViewer.h include/CInfoDialog.h include/CMDIWindow.h include/CMySQLServer.h include/CMyWindow.h include/CPlugin.h include/CPluginWindow.h include/CProperties.h include/CQueryTable.h include/CQueryWindow.h include/CQueryWindowOptionsDialog.h include/CRegisterServerDialog.h include/CServerAdministrationItem.h include/CServerItem.h include/CShowDatabaseGroupProperties.h include/CShowServerProperties.h include/CShowTableGroupProperties.h include/CShowTableProperties.h include/CSqlEditorFont.h include/CSqlTable.h include/CTable.h include/CTableFieldChooser.h include/CTableGroupItem.h include/CTableItem.h include/CTableProperties.h include/CTableSpecific.h include/CTableTools.h include/CTableWindow.h include/CTablesListBox.h include/CTextDialog.h include/CTextFieldEditor.h include/CToolBar.h include/CUserAdminItem.h include/CUserAdminWindow.h include/CUserItem.h include/config.h include/globals.h include/icons.h include/mysqlcc_main.h include/panels.h include/resource.h include/shared_menus.h include/CSqlEditor/CSqlEditor.h include/CSqlEditor/completion.h include/CSqlEditor/editor.h include/CSqlEditor/paragdata.h include/CSqlEditor/parenmatcher.h include/CSqlEditor/sqlcompletion.h include/CSqlEditor/syntaxhighliter_sql.h shared/include/CAction.h shared/include/CConfig.h shared/include/CMySQL.h shared/include/CMySQLCCPlugin.h shared/include/CMySQLQuery.h shared/include/CNullLineEdit.h shared/include/CSqlTableItem.h shared/include/shared.h





