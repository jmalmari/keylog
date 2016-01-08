cmake_minimum_required(VERSION 2.8.11)
project(KeyStats)

# Use C++11 features
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Sqlite3 REQUIRED)

add_executable(
  keystats
  src/main.cpp
  src/input/inputdevice.cpp
  src/log/keylog.cpp
)

target_link_libraries(
  keystats
  ${SQLITE3_LIBRARY}
)
