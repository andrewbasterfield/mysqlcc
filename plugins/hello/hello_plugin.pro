TEMPLATE = lib
CONFIG   = qt warn_on release plugin thread
DEFINES  += QT_DLL
DEFINES += MYSQLCC_PLUGIN
TARGET   = hello_plugin
LANGUAGE = C++
MOC_DIR  = moc

INCLUDEPATH    += ./../../shared/include \

win32 {
        LIBS           +=  C:\my\mysql\lib\opt\libmysql.lib
        INCLUDEPATH    +=  C:\my\mysql\include
        CONFIG         +=  windows
}

unix {
        LIBS           += -L/usr/local/mysql/lib -lmysqlclient -lz
        INCLUDEPATH    += /usr/local/mysql/include
        CONFIG         += x11 
        OBJECTS_DIR     = tmp
}


SOURCES  = hello_plugin.cpp \
	   ../../shared/src/CAction.cpp \
	   hello.cpp


HEADERS  = hello_plugin.h \
	   ../../shared/include/CAction.h \
	   ../../shared/include/CMySQLCCPlugin.h \
	   hello.h



