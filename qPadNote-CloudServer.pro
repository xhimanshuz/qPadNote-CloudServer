CONFIG -= qt
#CONFIG += console

DEFINES += SERVER

HEADERS += \
    include/Block.h \
    include/Protocol.h \
    include/cloudserver.h \
    include/mongocxxinterface.h \
    include/sessionhandler.h

SOURCES += \
    Run.cpp \
    src/Block.cpp \
    src/cloudserver.cpp \
    src/mongocxxinterface.cpp \
    src/sessionhandler.cpp

INCLUDEPATH += include \
               /usr/local/include/mongocxx/v_noabi \
               /usr/local/include/bsoncxx/v_noabi

LIBS += -L/usr/local/lib -lmongocxx -lbsoncxx

LIBS += -lpthread

DESTDIR = $$PWD/build

DISTFILES += build/setting.json \
             README.md
