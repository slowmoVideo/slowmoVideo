
set(SRCS
    testFlowRW_sV.cpp
    testFlowField_sV.cpp
    testAll.cpp
)
set(SRCS_MOC
    testFlowRW_sV.h
    testFlowField_sV.h
)

qt4_wrap_cpp(MOC_OUT ${SRCS_MOC})
#add_executable(testFlowRW testFlowRW_sV.cpp ${MOC_OUT})
#target_link_libraries(testFlowRW sVflow ${QT_LIBRARIES})

add_executable(unittests ${SRCS} ${MOC_OUT})
target_link_libraries(unittests sVflow ${QT_LIBRARIES})
