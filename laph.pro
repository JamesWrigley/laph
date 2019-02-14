######################################################################
# Automatically generated by qmake (3.1) Tue Nov 21 18:59:59 2017
######################################################################

QT += qml quick
TARGET = laph
TEMPLATE = app
LIBS += -L$$(JULIA_LIB_PATH) -ljulia
CONFIG += c++14 debug_and_release
INCLUDEPATH += . $$(JULIA_INCLUDE_PATH)
DEFINES += QT_DEPRECATED_WARNINGS JULIA_ENABLE_THREADING

# Input
SOURCES += src/main.cpp        \
           src/NodeMonitor.cpp \
           src/Glaph.cpp       \
           src/NodeItem.cpp    \
           src/WireItem.cpp    \
           src/SocketModel.cpp \
           src/util.cpp        \
           src/MessageModel.cpp

HEADERS += src/NodeMonitor.hpp \
           src/Glaph.hpp       \
           src/NodeItem.hpp    \
           src/WireItem.hpp    \
           src/Socket.hpp      \
           src/SocketModel.hpp \
           src/util.hpp        \
           src/XCom.hpp        \
           src/MessageModel.hpp
