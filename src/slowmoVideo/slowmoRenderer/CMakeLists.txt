
include_directories(..)

set(SRCS
    rendererMain.cpp
    slowmoRenderer_sV.cpp
)
set(SRCS_MOC
    slowmoRenderer_sV.h
)
qt4_wrap_cpp(MOC_OUT ${SRCS_MOC})

add_executable(slowmoRenderer ${SRCS} ${MOC_OUT})
target_link_libraries(slowmoRenderer sVproj ${EXTERNAL_LIBS})
install(TARGETS slowmoRenderer DESTINATION bin)
