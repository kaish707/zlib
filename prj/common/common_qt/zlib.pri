#   
# file:			zlib.pri
# created on:	2010 May 27 
# created by:	D. Kalantaryan (davit.kalantaryan@gmail.com)  
#  


message("file:  zlib.pri  ")
#include($${PWD}/../../common/common_qt/sys_common.pri)


win32:  QMAKE_CFLAGS += /FI"zlib_first_include.h"
else {
	QMAKE_CFLAGS += -include"zlib_first_include.h"
	QMAKE_CFLAGS += -Wimplicit-fallthrough=0
}

INCLUDEPATH += "$${PWD}/../../../include"
INCLUDEPATH += "$${PWD}/../../../contrib/zlib"

SOURCES		+=	\
	$${PWD}/../../../contrib/zlib/adler32.c			\
	$${PWD}/../../../contrib/zlib/compress.c		\
	$${PWD}/../../../contrib/zlib/crc32.c			\
	$${PWD}/../../../contrib/zlib/deflate.c			\
	$${PWD}/../../../contrib/zlib/gzclose.c			\
	$${PWD}/../../../contrib/zlib/gzlib.c			\
	$${PWD}/../../../contrib/zlib/gzread.c			\
	$${PWD}/../../../contrib/zlib/gzwrite.c			\
	$${PWD}/../../../contrib/zlib/infback.c			\
	$${PWD}/../../../contrib/zlib/inffast.c			\
	$${PWD}/../../../contrib/zlib/inflate.c			\
	$${PWD}/../../../contrib/zlib/inftrees.c		\
	$${PWD}/../../../contrib/zlib/trees.c			\
	$${PWD}/../../../contrib/zlib/uncompr.c			\
	$${PWD}/../../../contrib/zlib/zutil.c			

HEADERS		+=	\
	$${PWD}/../../../include/zlib_first_include.h
