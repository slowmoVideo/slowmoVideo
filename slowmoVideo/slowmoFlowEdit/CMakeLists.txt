
include_directories(${slowmoVideo_SOURCE_DIR})

set(SRCS
    main.cpp
    mainwindow.cpp
    flowEditCanvas.cpp
)

set(SRCS_UI
    mainwindow.ui
    flowEditCanvas.ui
)


set(SRCS_MOC
    mainwindow.h
    flowEditCanvas.h
)

qt4_wrap_ui(UI_H_OUT ${SRCS_UI})
qt4_wrap_cpp(MOC_OUT ${SRCS_MOC})

include_directories(${slowmoVideo_SOURCE_DIR}/libgui)
include_directories(${CMAKE_BINARY_DIR}/slowmoFlowEdit)

add_executable(slowmoFlowEdit ${SRCS} ${MOC_OUT} ${UI_H_OUT})
target_link_libraries(slowmoFlowEdit sVgui sVflow sVvis ${QT_LIBRARIES})

install(TARGETS slowmoFlowEdit DESTINATION bin)
