
set(SRCS
    testFlowRW_sV.cpp
    testFlowField_sV.cpp
    testVector_sV.cpp
    testIntMatrix_sV.cpp
    testXmlProjectRW_sV.cpp
    testDefs_sV.cpp
    testShutterFunction_sV.cpp
    testProject_sV.cpp
    testNodeList_sV.cpp
    testAll.cpp
)

include_directories(${FFMPEG_INCLUDE_PATHS})
add_executable(UnitTests ${SRCS})
qt5_use_modules(UnitTests Concurrent  Core Test)
target_link_libraries(UnitTests sVproj ${EXTERNAL_LIBS})

add_test(UnitTests UnitTests)
