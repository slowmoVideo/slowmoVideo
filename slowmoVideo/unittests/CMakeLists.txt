
set(SRCS_MOC
    testFlowRW_sV.h
)

qt4_wrap_cpp(MOC_OUT ${SRCS_MOC})
add_executable(testFlowRW testFlowRW_sV.cpp ${MOC_OUT})
target_link_libraries(testFlowRW sVflow ${QT_LIBRARIES})
