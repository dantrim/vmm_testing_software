QT -= gui
QT += network

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += src/
INCLUDEPATH += include/
INCLUDEPATH += external/

DEPENDPATH += external/
DEPENDPATH += external/spdlog

OBJECTS_DIR += ./objects/
MOC_DIR += ./moc/
RCC_DIR += ./rcc/
UI_DIR += ./ui/

SOURCES += \
        src/main.cpp \
        src/helpers.cpp \
        src/vts_server.cpp \
        src/vts_commands.cpp \
        src/communicator_frontend.cpp \
        src/spi_builder.cpp \
        src/frontend_address.cpp \
        src/frontend_struct.cpp
HEADERS += \
        include/vts_server.h \
        include/helpers.h \
        include/vts_message.h \
        include/vts_commands.h \
        include/communicator_frontend.h \
        include/spi_builder.h \
        include/frontend_address.h \
        include/frontend_struct.h \
        # external
        external/nlohmann/json.hpp \
        external/spdlog/spdlog.h

TARGET = vts_server

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
