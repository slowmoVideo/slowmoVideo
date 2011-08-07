set(SRCS
    imageDisplay.cpp
    combinedShortcuts.cpp
)

set(SRCS_MOC
    imageDisplay.h
    combinedShortcuts.h
)


qt4_wrap_cpp(MOC_OUT ${SRCS_MOC})


include_directories(${CMAKE_BINARY_DIR}/libgui)

add_library(sVgui STATIC ${SRCS} ${MOC_OUT})
target_link_libraries(sVgui  ${QT_LIBRARIES})

# If the library is used in a different project, cmake requires:
#include_directories(${slowmoVideo_SOURCE_DIR}/libgui)
