
#find_package(Qt4 REQUIRED)
#include(${QT_USE_FILE})
#include_directories(${QT_INCLUDES})


set(LIB_SRC
  interpolate_sV.cpp
  project_sV.cpp
)

add_library(sV STATIC ${LIB_SRC})
target_link_libraries(sV ${QT_LIBRARIES})