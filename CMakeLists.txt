cmake_minimum_required(VERSION 3.0)
project(router-simulation C)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set (CMAKE_C_STANDARD 99)

add_library(sources
  src/terminal.c
  src/receiver.c
  src/sender.c
  src/packet_handler.c
)
include_directories(include)

#configure_file(test_kernel.cl test_kernel.cl COPYONLY)

add_executable(Router app/router.c)
target_link_libraries(Router
  sources
  pthread
)
