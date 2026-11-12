HEADERS += $$PWD/win9x_opl_proxy.h
SOURCES += $$PWD/win9x_opl_proxy.cpp

linux* {
    LIBS += -ldl
}
