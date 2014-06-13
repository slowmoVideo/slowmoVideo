set(SRCS
    imageDisplay.cpp
    combinedShortcuts.cpp
)

set(SRCS_MOC
    imageDisplay.h
    combinedShortcuts.h
)


qt_wrap_cpp(MOC_OUT ${SRCS_MOC})


include_directories(${CMAKE_BINARY_DIR}/libgui)

add_library(sVgui STATIC ${SRCS} ${MOC_OUT})
qt_use_modules(sVgui Core)
qt_use_modules(sVgui Gui)
qt_use_modules(sVgui Widgets)
target_link_libraries(sVgui  ${EXTERNAL_LIBS})

# If the library is used in a different project, cmake requires:
#include_directories(${slowmoVideo_SOURCE_DIR}/libgui)
