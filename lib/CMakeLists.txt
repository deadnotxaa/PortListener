add_library(listen ListenPort.hpp ListenPort.cpp)

if (APPLE)
    target_link_libraries(listen PRIVATE ${Boost_LIBRARIES})
elseif (WIN32)
    target_link_libraries(listen PRIVATE ${Boost_LIBRARIES} stdc++fs ws2_32)
endif ()