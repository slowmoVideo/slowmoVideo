# Docs: http://www.cmake.org/cmake/help/cmake-2-8-docs.html

cmake_minimum_required(VERSION 2.8)

project(slowmoVideo)

set(CMAKE_INSTALL_PREFIX
  ../..
)


find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
include_directories(${QT_INCLUDES})


add_subdirectory(lib)
add_subdirectory(slowmoCLI)
add_subdirectory(slowmoUI)
add_subdirectory(test)

set(CMAKE_BUILD_TYPE Debug)

# To compile with clang:
#set(CMAKE_CXX_COMPILER "clang++")

set(CMAKE_CXX_FLAGS
  ${CMAKE_CXX_FLAGS}
  -Wall
)


include(cmake/macros.cmake)
copy_files(
  fetch.sh
)
install(FILES fetch.sh DESTINATION bin PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)