TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += `pkg-config --cflags jack`
LIBS += `pkg-config --libs jack` -lrt -lm -lpthread -lsndfile

SOURCES += main.cpp \
    jackclient.cpp \
    netclient.cpp \
    sender.cpp \
    soundfile.cpp \
    c-common/byte-order.c \
    c-common/client.c \
    c-common/file.c \
    c-common/jack-client.c \
    c-common/jack-port.c \
    c-common/jack-ringbuffer.c \
    c-common/memory.c \
    c-common/network.c \
    c-common/sound-file.c \
    c-common/time-timeval.c

HEADERS += \
    c-common/byte-order.h \
    c-common/client.h \
    c-common/failure.h \
    c-common/file.h \
    c-common/float.h \
    c-common/int.h \
    c-common/jack-client.h \
    c-common/jack-port.h \
    c-common/jack-ringbuffer.h \
    c-common/memory.h \
    c-common/network.h \
    c-common/print.h \
    c-common/sound-file.h \
    c-common/time-timeval.h \
    jackclient.h \
    netclient.h \
    sender.h \
    soundfile.h

DISTFILES += \
    c-common/README
