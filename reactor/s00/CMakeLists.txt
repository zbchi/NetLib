cmake_minimum_required(VERSION 3.10)
project(NetLib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_ROOT ${CMAKE_SOURCE_DIR}/../..)

include_directories(
    ${PROJECT_ROOT}/base
    ${CMAKE_SOURCE_DIR}
)


# 自动收集源文件
file(GLOB BASE_SOURCES ${PROJECT_ROOT}/base/*.cc)
file(GLOB REACTOR_SOURCES 
${CMAKE_SOURCE_DIR}/main.cc
)


add_executable(test ${BASE_SOURCES} ${REACTOR_SOURCES})
target_link_libraries(test PRIVATE pthread)