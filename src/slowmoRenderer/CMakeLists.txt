
include_directories(..)

set(SRCS
    rendererMain.cpp
    slowmoRenderer_sV.cpp
)

add_executable(slowmoRenderer ${SRCS})
target_link_libraries(slowmoRenderer sVproj ${EXTERNAL_LIBS})
qt5_use_modules(slowmoRenderer Script Widgets Concurrent Gui  Core )
install(TARGETS slowmoRenderer DESTINATION ${DEST})
