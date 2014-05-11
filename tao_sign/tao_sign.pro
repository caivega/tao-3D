# ******************************************************************************
#  tao_sign.pro                                                     Tao project
# ******************************************************************************
# File Description:
# 
#    Qt program description for the Tao licence signing utility
#
#
#
#
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2011 Christophe de Dinechin <christophe@taodyne.com>
# (C) 2011 Jérôme Forissier <jerome@taodyne.com> 
# (C) 2011 Taodyne SAS
# ******************************************************************************

include(../main.pri)

TEMPLATE = app
TARGET   = tao_sign
SOURCES += sign.cpp ../tao/license.cpp ../tao/crypto.cpp
HEADERS += ../tao/license.h
CONFIG  += console static
CONFIG  -= app_bundle
QT      -= gui
DEFINES += KEYGEN

# Linux release mode: strip binary, ready for installation on web shop server
# ("make install" normally does it, but there is no install for tao_sign)
linux-g++*:CONFIG(release, debug|release):MAYBE_STRIP_CMD=\$(STRIP) $$TARGET;

# Turn off warnings caused by Crypto++ headers (unused parameters)
CONFIG += warn_off
QMAKE_CXXFLAGS -= -Werror

INC = . ../tao ../tao/include ../tao/include/tao ../tao/xlr/xlr/include \
      ../libcryptopp ../keygen
DEPENDPATH += $$INC
INCLUDEPATH += $$INC
LIBS += -L../libxlr/\$(DESTDIR) -lxlr -L../libcryptopp/\$(DESTDIR) -lcryptopp
# Windows needs ws2_32.dll for ntohs() due to tao/crypto.cpp
win32:LIBS += -lws2_32

# Automatic embedding of Git version
QMAKE_CLEAN += version.h
PRE_TARGETDEPS += version.h
revtarget.target = version.h
revtarget.commands = ../tao/updaterev.sh
revtarget.depends = $$SOURCES \
    $$HEADERS
QMAKE_EXTRA_TARGETS += revtarget

# Convenience script to run signing program
macx:SIGN_CMD  = export DYLD_LIBRARY_PATH=$$PWD/../libxlr ; $$PWD/tao_sign \\\"\\\$$@\\\"
linux-g++*:SIGN_CMD = export LD_LIBRARY_PATH=$$PWD/../libxlr ; $$PWD/tao_sign \\\"\\\$$@\\\"
win32 {
  HERE = $$system(bash -c 'pwd | sed \'s@\\([a-zA-Z]\\):@/\\1@\'')
  # This is one of the most convoluted script/make/qmake line I've ever written...
  SIGN_CMD = export PATH=\\\"\\\$$PATH:$$HERE/../libxlr/\\\"\$(DESTDIR); \\\"$$HERE/\\\"\$(DESTDIR_TARGET) \\\"\\\$$@\\\"
}
QMAKE_CLEAN += tao_sign.sh
QMAKE_POST_LINK = $$MAYBE_STRIP_CMD echo \'$$SIGN_CMD\' > tao_sign.sh && chmod +x tao_sign.sh  # Does not really belong to post-link, but it works

include(../make_install_kludge.pri)
