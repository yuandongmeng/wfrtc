
CONFIG += c++11

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
#vc

HOME_BASE=../
INCLUDEPATH += $$HOME_BASE/include

unix{
    CONFIG(debug, debug|release) {
        LIBS += -L$$HOME_BASE/bin/lib_debug
        DESTDIR += $$HOME_BASE/bin/app_debug
    }else{
        LIBS += -L$$HOME_BASE/bin/lib_release
        DESTDIR += $$HOME_BASE/bin/app_release
    }

 LIBS += -L$$HOME_BASE/thirdparty/lib -lYVRTCEngine  -lYVRTCCore  -lyuv -lopus -lopenh264 -lspeexdsp -lusrsctp -lpthread -lasound -ldl
#openssl
 LIBS += -lssl2 -lcrypto2 -lsrtp2
#gmssl
 #LIBS += -lssl_gm -lcrypto_gm -lyvrsrtp3

}

win32{
    CONFIG(debug, debug|release) {
        LIBS += -L$$HOME_BASE/bin/lib_win_debug
        DESTDIR += $$HOME_BASE/bin/app_win_debug
    }else{
        LIBS += -L$$HOME_BASE/bin/lib_win_release
        DESTDIR += $$HOME_BASE/bin/app_win_release
    }
    LIBS += -lYVRTCEngine  -lYVRTCCore -lyuv -lopus -lopenh264  -lspeexdsp -lavutil -lavcodec -lusrsctp -lksuser -lwinmm  -lole32 -lStrmiids

    THIRD_LIB=  -L$$HOME_BASE/thirdparty/lib/win -lsrtp2  -lssl  -lcrypto
    msvc{
        #QMAKE_CFLAGS += /utf-8
        #QMAKE_CXXFLAGS += /utf-8
        QMAKE_LFLAGS    += /ignore:4099
        DEFINES +=HAVE_STRUCT_TIMESPEC
        DEFINES +=WIN32_LEAN_AND_MEAN
        INCLUDEPATH += $$HOME_BASE\thirdparty\include\win\include   #vc
        THIRD_LIB=  -L$$HOME_BASE/thirdparty/lib/win/msvc -lpthreadVC2 -lavrt -luser32 -lAdvapi32
        #openssl
        THIRD_LIB+= -lsrtp2  -llibcrypto -llibssl
        #gmssl
        QMAKE_CXXFLAGS += /source-charset:utf-8 /execution-charset:utf-8
    }

    LIBS +=  $$THIRD_LIB
    LIBS +=   -lCrypt32 -lws2_32
}

SOURCES += \
    main.cpp


 HEADERS += \
     yvrtc_interface.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
