# Locate Apple QTKit (next-generation QuickTime)
# This module defines
# QTKIT_LIBRARY
# QTKIT_FOUND, if false, do not try to link to gdal 
# QTKIT_INCLUDE_DIR, where to find the headers
#
# $QTKIT_DIR is an environment variable that would
# correspond to the ./configure --prefix=$QTKIT_DIR
#
# Created by Eric Wing. 

# QTKit on OS X looks different than QTKit for Windows,
# so I am going to case the two.

IF(APPLE)
  FIND_PATH(QTKIT_INCLUDE_DIR QTKit/QTKit.h)
  FIND_LIBRARY(QTKIT_LIBRARY QTKit)
ENDIF()


SET(QTKIT_FOUND "NO")
IF(QTKIT_LIBRARY AND QTKIT_INCLUDE_DIR)
  SET(QTKIT_FOUND "YES")
ENDIF()

