cmake_minimum_required(VERSION 3.1)
project(havka)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREAD ON)
find_package(Boost 1.71.0 REQUIRED COMPONENTS system thread regex)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  link_directories(${Boost_LIBRARY_DIRS})
endif(Boost_FOUND)
set(BOOST_LIBS ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_REGEX_LIBRARY})


find_package(yaml-cpp REQUIRED)
include_directories(${YAML_CPP_INCLUDE_DIRS})


find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})


include_directories(src)


add_executable(server server.cpp
        src/server/server.cpp
        src/server/net.cpp
        src/server/queue.cpp
        src/server/server_config.cpp
        src/server/storage.cpp
        src/util.cpp
        )
set_target_properties(server PROPERTIES COMPILE_FLAGS "-DMONITORING")
target_link_libraries(server ${BOOST_LIBS} ${YAML_CPP_LIBRARIES})


add_executable(client client.cpp
        src/client/client.cpp
        src/client/client_config.cpp
        src/util.cpp
        )
set_target_properties(client PROPERTIES COMPILE_FLAGS "-DMONITORING")
target_link_libraries(client ${BOOST_LIBS} ${YAML_CPP_LIBRARIES})


add_executable(test tests/main.cpp
                    tests/ConfigTest.cpp
                    tests/QueueTest.cpp
                    tests/StorageTest.cpp
                    tests/IntegrationTests.cpp
        src/server/server.cpp
        src/server/net.cpp
        src/server/queue.cpp
        src/server/server_config.cpp
        src/client/client_config.cpp
        src/server/storage.cpp
        src/client/client.cpp
        src/util.cpp
        )
target_link_libraries(test ${BOOST_LIBS} ${YAML_CPP_LIBRARIES} ${GTEST_LIBRARIES})