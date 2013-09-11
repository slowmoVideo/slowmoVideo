# Source: http://openlibraries.org/browser/trunk/FindGLEW.cmake (LGPL)
# - Try to find GLEW
# Once done this will define
#  
#  GLEW_FOUND        - system has GLEW
#  GLEW_INCLUDE_DIR  - the GLEW include directory
#  GLEW_LIBRARIES_DIR  - where the libraries are
#  GLEW_LIBRARIES      - Link these to use GLEW
#   

IF (GLEW_INCLUDE_DIR)
  # Already in cache, be silent
  SET(GLEW_FIND_QUIETLY TRUE)
ENDIF (GLEW_INCLUDE_DIR)

if( WIN32 )
   if( MSVC80 )
       set( COMPILER_PATH "C:/Program\ Files/Microsoft\ Visual\ Studio\ 8/VC" )
   endif( MSVC80 )
   if( MSVC71 )
       set( COMPILER_PATH "C:/Program\ Files/Microsoft\ Visual\ Studio\ .NET\ 2003/Vc7" )
   endif( MSVC71 )
   FIND_PATH( GLEW_INCLUDE_DIR gl/glew.h gl/wglew.h
              PATHS c:/glew/include ${COMPILER_PATH}/PlatformSDK/Include ${PROJECT_SOURCE_DIR}/libs/include)
   SET( GLEW_NAMES glew32 )
   FIND_LIBRARY( GLEW_LIBRARIES
                 NAMES ${GLEW_NAMES}
                 PATHS
				 c:/glew/lib 
				 ${COMPILER_PATH}/PlatformSDK/Lib
				 ${PROJECT_SOURCE_DIR}/libs/lib
	)
	message(STATUS "GLEW library: ${GLEW_LIBRARIES} at ${GLEW_INCLUDE_DIR} (also searched at ${PROJECT_SOURCE_DIR}/libs/include)")
else( WIN32 )
   FIND_PATH( GLEW_INCLUDE_DIR glew.h wglew.h
              PATHS /usr/local/include /usr/include
              PATH_SUFFIXES gl/ GL/ )
   SET( GLEW_NAMES glew GLEW )
   FIND_LIBRARY( GLEW_LIBRARIES
                 NAMES ${GLEW_NAMES}
                 PATHS /usr/lib /usr/local/lib )
endif( WIN32 )

GET_FILENAME_COMPONENT( GLEW_LIBRARIES_DIR ${GLEW_LIBRARIES} PATH )

IF (GLEW_INCLUDE_DIR AND GLEW_LIBRARIES)
   SET(GLEW_FOUND TRUE)
    SET( GLEW_LIBRARIES_DIR ${GLEW_LIBRARIES} )
ELSE (GLEW_INCLUDE_DIR AND GLEW_LIBRARIES)
   SET( GLEW_FOUND FALSE )
   SET( GLEW_LIBRARIES_DIR )
ENDIF (GLEW_INCLUDE_DIR AND GLEW_LIBRARIES)

