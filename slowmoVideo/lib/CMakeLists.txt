# Static libraries
# http://www.itk.org/pipermail/insight-users/2007-November/024141.html
# http://www.linux-magazin.de/Heft-Abo/Ausgaben/2007/02/Mal-ausspannen

set(LIB_SRC
  interpolate_sV.cpp
  project_sV.cpp
)

add_library(sV STATIC ${LIB_SRC})
target_link_libraries(sV ${QT_LIBRARIES})