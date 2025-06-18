QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DEFINES += USE_OPENSSL
INCLUDEPATH += "C:/Users/hamma/vcpkg/installed/x64-windows/include"
SOURCES += \
    main.cpp \
    securex.cpp

HEADERS += \
    detect_usb.h \
    encrypt_bound_with_usb.h \
    file_encryption_algo.h \
    image_stegnography.h \
    password_generator.h \
    securex.h \
    stb_image.h \
    stb_image_write.h \
    timelock.h
LIBS += -lsetupapi
LIBS += -L"C:/Users/hamma/vcpkg/installed/x64-windows/lib" -llibcrypto -llibssl

FORMS += \
    securex.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
