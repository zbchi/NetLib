cmake_minimum_required(VERSION 3.10)
project(NetLib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)


set(PROJECT_ROOT ${CMAKE_SOURCE_DIR}/..)

include_directories(
    ${PROJECT_ROOT}/base
    ${PROJECT_ROOT}/reactor
)


file(GLOB BASE_SOURCES ${PROJECT_ROOT}/base/*.cc )
file(GLOB REACTOR_SOURCES ${PROJECT_ROOT}/reactor/*.cc)
file(GLOB TEST_SOURCES ${CMAKE_SOURCE_DIR}/test13.cc)

add_executable(test ${BASE_SOURCES} ${REACTOR_SOURCES} ${TEST_SOURCES})
target_link_libraries(test PRIVATE pthread)