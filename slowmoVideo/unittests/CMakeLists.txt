
set(SRCS
    testFlowRW_sV.cpp
    testFlowField_sV.cpp
    testVector_sV.cpp
    testIntMatrix_sV.cpp
    testXmlProjectRW_sV.cpp
    testDefs_sV.cpp
    testAll.cpp
)
set(SRCS_MOC
    testFlowRW_sV.h
    testFlowField_sV.h
    testVector_sV.h
    testIntMatrix_sV.h
    testDefs_sV.h
    testXmlProjectRW_sV.h
)

qt4_wrap_cpp(MOC_OUT ${SRCS_MOC})

add_executable(unittests ${SRCS} ${MOC_OUT})
target_link_libraries(unittests sVproj ${QT_LIBRARIES})
