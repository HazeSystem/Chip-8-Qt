QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
TARGET = chip8-qt

DESTDIR = $$PWD/bin

HEADERS += SDL2Widget.h \
    c8dasm/c8dasm.h \
    c8dasm/disassembler.h \
    chip8.h \
    debugger.h \
    hexview/document/buffer/qfilebuffer.h \
    hexview/document/buffer/qhexbuffer.h \
    hexview/document/buffer/qmemorybuffer.h \
    hexview/document/buffer/qmemoryrefbuffer.h \
    hexview/document/commands/hexcommand.h \
    hexview/document/commands/insertcommand.h \
    hexview/document/commands/removecommand.h \
    hexview/document/commands/replacecommand.h \
    hexview/document/qhexcursor.h \
    hexview/document/qhexdocument.h \
    hexview/document/qhexmetadata.h \
    hexview/document/qhexrenderer.h \
    hexview/qhexview.h \
    mainwindow.h

SOURCES += main.cpp \
           SDL2Widget.cpp \
           c8dasm/c8dasm.cpp \
           c8dasm/disassembler.cpp \
           chip8.cpp \
           debugger.cpp \
           hexview/document/buffer/qfilebuffer.cpp \
           hexview/document/buffer/qhexbuffer.cpp \
           hexview/document/buffer/qmemorybuffer.cpp \
           hexview/document/buffer/qmemoryrefbuffer.cpp \
           hexview/document/commands/hexcommand.cpp \
           hexview/document/commands/insertcommand.cpp \
           hexview/document/commands/removecommand.cpp \
           hexview/document/commands/replacecommand.cpp \
           hexview/document/qhexcursor.cpp \
           hexview/document/qhexdocument.cpp \
           hexview/document/qhexmetadata.cpp \
           hexview/document/qhexrenderer.cpp \
           hexview/qhexview.cpp \
           mainwindow.cpp


LIBS += -L$$PWD/ThirdParty/lib/ -lSDL2

INCLUDEPATH += $$PWD/ThirdParty/include/SDL2
DEPENDPATH += $$PWD/ThirdParty/include/SDL2

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    debugger.ui \
    mainwindow.ui

RESOURCES +=
