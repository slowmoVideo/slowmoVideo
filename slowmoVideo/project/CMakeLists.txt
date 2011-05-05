set(SRCS_PROJ
  flow_sV.cpp
  project_sV.cpp
)

set(SRCS_MOC
  flow_sV.h
  project_sV.h
)

qt4_wrap_cpp(MOC_OUT ${SRCS_MOC})

add_library(sVproj STATIC ${SRCS_PROJ} ${MOC_OUT})
target_link_libraries(sVproj sV ${QT_LIBRARIES})
