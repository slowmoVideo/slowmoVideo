
# Building Qt+UI apps:
# http://www.qtcentre.org/wiki/index.php?title=Compiling_Qt4_apps_with_CMake
# http://www.cmake.org/pipermail/cmake/2008-September/023908.html



include_directories(${slowmoVideo_SOURCE_DIR})

set(SRCS
  main.cpp
  mainwindow.cpp
  canvas.cpp
  canvasTools.cpp
  flowEditCanvas.cpp
  frameMonitor.cpp
  renderPreview.cpp
  dialogues/newProjectDialog.cpp
  dialogues/preferencesDialog.cpp
  dialogues/projectPreferencesDialog.cpp
  dialogues/progressDialog.cpp
  dialogues/renderingDialog.cpp
  dialogues/shutterFunctionDialog.cpp
  dialogues/shutterFunctionFrame.cpp
  dialogues/flowExaminer.cpp
  dialogues/tagAddDialog.cpp
  dialogues/aboutDialog.cpp
  notificator.cpp
	logbrowserdialog.cpp
)

# notification
if (APPLE)
find_library(CORE_FOUNDATION NAMES CoreFoundation)
find_library(FOUNDATION NAMES Foundation)
find_library(APPLICATION_SERVICES NAMES ApplicationServices)
find_library(CORE_SERVICES NAMES CoreServices)
find_library(COCOA NAMES Cocoa)

SET(OSX_EXTRA_LIBS ${FOUNDATION} ${CORE_FOUNDATION} ${APPLICATION_SERVICES})
message(STATUS "OSX Additional libraries: ${OSX_EXTRA_LIBS}")

set(SRCS
        ${SRCS}
        macnotificationhandler.mm
)
endif()

set(SRCS_UI
  mainwindow.ui
  canvas.ui
  frameMonitor.ui
  renderPreview.ui
  flowEditCanvas.ui
  dialogues/newProjectDialog.ui
  dialogues/preferencesDialog.ui
  dialogues/projectPreferencesDialog.ui
  dialogues/progressDialog.ui
  dialogues/renderingDialog.ui
  dialogues/shutterFunctionDialog.ui
  dialogues/flowExaminer.ui
  dialogues/tagAddDialog.ui
  dialogues/aboutDialog.ui
)

set(SRCS_MOC
  mainwindow.h
  canvas.h
  frameMonitor.h
  flowEditCanvas.h
  renderPreview.h
  dialogues/newProjectDialog.h
  dialogues/preferencesDialog.h
  dialogues/projectPreferencesDialog.h
  dialogues/progressDialog.h
  dialogues/renderingDialog.h
  dialogues/shutterFunctionDialog.h
  dialogues/shutterFunctionFrame.h
  dialogues/flowExaminer.h
  dialogues/tagAddDialog.h
  dialogues/aboutDialog.h
  notificator.h
	logbrowserdialog.h
)

# notification
if (APPLE)
set(SRCS_MOC
        ${SRCS_MOC}
        macnotificationhandler.h
)
endif()

if(APPLE)
	set(BUNDLE "slowmoUI")
	set(ICONS_DIR "${${PROJECT_NAME}_SOURCE_DIR}/slowmoVideo/slowmoUI/res")
        message( "OS X build" )
        set(MACOSX_BUNDLE_INFO_STRING "${BUNDLE} ${PROJECT_VERSION}")
        set(MACOSX_BUNDLE_BUNDLE_VERSION "${BUNDLE} ${PROJECT_VERSION}")
        set(MACOSX_BUNDLE_LONG_VERSION_STRING "${BUNDLE} ${PROJECT_VERSION}")
        set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
        set(MACOSX_BUNDLE_COPYRIGHT ${COPYRIGHT} )
        set(MACOSX_BUNDLE_ICON_FILE "slowmoUI.icns")
        set(MACOSX_BUNDLE_GUI_IDENTIFIER ${IDENTIFIER} )
        set(MACOSX_BUNDLE_BUNDLE_NAME "${BUNDLE}")

        set(MACOSX_BUNDLE_RESOURCES "${CMAKE_CURRENT_BINARY_DIR}/${BUNDLE}.app/Contents/Resources")
        set(MACOSX_BUNDLE_ICON "${ICONS_DIR}/${MACOSX_BUNDLE_ICON_FILE}")
	SET_SOURCE_FILES_PROPERTIES(
	   ${MACOSX_BUNDLE_ICON} 
	    PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
        message(STATUS "slowmoUI Bundle will be : ${MACOSX_BUNDLE} ")

	set( SRCS ${SRCS} ${MACOSX_BUNDLE_ICON} )

  set(INFO_PLIST_FILENAME ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
  configure_file(${CMAKE_SOURCE_DIR}/slowmoVideo/slowmoUI/res/Info.plist.in ${INFO_PLIST_FILENAME})
  set_target_properties(${PROGNAME} PROPERTIES
                        MACOSX_BUNDLE_INFO_PLIST ${INFO_PLIST_FILENAME} )
  #ADD_CUSTOM_COMMAND(TARGET slowmoUI
  #  POST_BUILD
  #  COMMAND cp Info.plist slowmoUI.app/Contents/Info.plist
  #  COMMENT "Updating Info.plist"
  #  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )

endif()

# Without these includes the promoted widgets fail to compile
# since the headers are not found. (For whatever reason.)
include_directories(dialogues)
include_directories(.)
include_directories(..)

# Embed images in the binary
set(SRC_RES resources.qrc)
qt_add_resources(RES_OUT ${SRC_RES})

# Generate header files from the .ui files
qt_wrap_ui(UI_H_OUT ${SRCS_UI})
qt_wrap_cpp(MOC_OUT ${SRCS_MOC})

# Include the generated header files
include_directories(${CMAKE_BINARY_DIR}/slowmoVideo/slowmoUI)


link_directories(${FFMPEG_LIBRARY_DIR})
add_executable(slowmoUI WIN32 MACOSX_BUNDLE ${SRCS} ${MOC_OUT} ${UI_H_OUT} ${RES_OUT})
qt_use_modules(slowmoUI Script Widgets Concurrent Gui  Core )
target_link_libraries(slowmoUI sVproj sVvis sVgui ${OSX_EXTRA_LIBS})

#install(TARGETS ${slowmoUI}
#        BUNDLE DESTINATION . COMPONENT Runtime
#        RUNTIME DESTINATION ${BIN_INSTALL_DIR} COMPONENT Runtime)

if (UNIX AND NOT APPLE)
# create desktop file for KDE/Gnome
# desktop file section
	file( WRITE "${PROJECT_BINARY_DIR}/slowmoUI.desktop"
"#!/usr/bin/env xdg-open
[Desktop Entry]
Type=Application
Comment=Slow Motion Video
Exec=${CMAKE_INSTALL_PREFIX}/bin/slowmoUI
GenericName=slowmoVideo
Icon=${CMAKE_INSTALL_PREFIX}/share/icons/AppIcon.png
MimeType=
Name=slowmoUI
Terminal=false
Categories=Qt;AudioVideo;Video;\n")

	#install ( FILES icons/slowmoUI.png DESTINATION share/icons )
	install ( FILES res/AppIcon.png DESTINATION share/icons )
	install ( FILES ${PROJECT_BINARY_DIR}/slowmoUI.desktop DESTINATION share/applications
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
endif()

if (Qt5Core_FOUND)
	include(DeployQt5) # 2.8.7 or later
else()
	include(DeployQt4) # 2.8.7 or later
endif()

#set( PLUGINS "qjpeg;qtiff" )
set( PLUGINS qjpeg )
if (APPLE)
       install(TARGETS slowmoUI DESTINATION ".")
       install_qt_executable(slowmoUI.app "${PLUGINS}"  )
elseif(WIN32)
       install(TARGETS slowmoUI DESTINATION ".")
       install_qt_executable(slowmoUI.exe "${PLUGINS}"  )
else()
       install(TARGETS slowmoUI DESTINATION ${DEST})
#       install_qt_executable(slowmoUI "${PLUGINS}"  )
endif()

