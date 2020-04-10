set(SRCS
    imageDisplay.cpp
    combinedShortcuts.cpp
)


include_directories(
        ${CMAKE_BINARY_DIR}/libgui
        ../lib/libsvflow
)

add_library(sVgui STATIC ${SRCS})
qt5_use_modules(sVgui Core)
qt5_use_modules(sVgui Gui)
qt5_use_modules(sVgui Widgets)
target_link_libraries(sVgui  ${EXTERNAL_LIBS})

# If the library is used in a different project, cmake requires:
#include_directories(${slowmoVideo_SOURCE_DIR}/libgui)
