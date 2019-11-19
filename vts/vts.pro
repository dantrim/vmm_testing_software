QT -= gui
QT += network

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

boostinclude=/usr/local/Cellar/boost160/1.60.0/include
boostlib=/usr/local/Cellar/boost160/1.60.0/lib

DEFINES += BOOST_ALL_NO_LIB

INCLUDEPATH += src/
INCLUDEPATH += include/
INCLUDEPATH += $$boostinclude
INCLUDEPATH += external/

DEPENDPATH += $$boostinclude
DEPENDPATH += external/
DEPENDPATH += external/spdlog

OBJECTS_DIR += ./objects/
MOC_DIR += ./moc/
RCC_DIR += ./rcc/
UI_DIR += ./ui/

linux {
    QMAKE_RPATHDIR += $$boostlib
    QMAKE_RPATHDIR += ./objects
}

LIBS +=  -L$$boostlib -lboost_thread-mt -lboost_filesystem-mt  -lboost_system-mt -lboost_chrono-mt -lboost_atomic-mt
#LIBS += -L$$boostlib -lboost_thread-mt -lboost_filesystem-mt  -lboost_system-mt 

SOURCES += \
        src/main.cpp \
        src/helpers.cpp \
        src/vts_server.cpp \
        src/vts_commands.cpp \
        src/communicator_frontend.cpp \
        src/spi_builder.cpp \
        src/frontend_address.cpp \
        src/frontend_struct.cpp \
        src/vts_result.cpp \
        src/vts_test_types.cpp \
        src/vts_test_handler.cpp \
        src/vts_test_states.cpp \
        # tests
        src/vts_test.cpp \
        src/vts_test_imp.cpp \
        # daq
        src/daq_handler.cpp \
        src/daq_listener.cpp \
        src/daq_data_builder.cpp \
        # decode
        src/vts_decode.cpp
HEADERS += \
        include/vts_server.h \
        include/helpers.h \
        include/vts_message.h \
        include/vts_commands.h \
        include/communicator_frontend.h \
        include/spi_builder.h \
        include/frontend_address.h \
        include/frontend_struct.h \
        include/vts_result.h \
        include/vts_test_types.h \
        include/vts_test.h \
        include/vts_test_imp.h \
        include/vts_test_handler.h \
        include/vts_test_states.h \
        # daq
        include/daq_data_fragment.h \
        include/daq_defs.h \
        include/daq_handler.h \
        include/daq_listener.h \
        include/daq_data_builder.h \
        # decode
        include/vts_decode.h \
        # external
        external/nlohmann/json.hpp \
        external/spdlog/spdlog.h \
        external/concurrentqueue/concurrentqueue.h
        

testsources = $$files(src/tests/*.cpp)
testheaders = $$files(include/tests/*.h)
SOURCES += $$testsources
HEADERS += $$testheaders
message($$testsources)
message($$testheaders)

TARGET = vts_server

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
