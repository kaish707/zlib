#   
# file:			zlib_test.pro  
# created on:	2010 May 27 
# created by:	D. Kalantaryan (davit.kalantaryan@gmail.com)  
#  


message("file:  zlib_test.pro  ")
include($${PWD}/../../common/common_qt/zlib.pri)


SOURCES		+=	\
	\
	$${PWD}/../../../src/tests/main_zlib_test.cpp
