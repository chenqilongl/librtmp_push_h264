cmake_minimum_required(VERSION 3.24)

project(demo_push_h264 )

add_executable(demo_push_h264
    simplest_librtmp_send264.cpp
    librtmp_send264.cpp)
    
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNO_CRYPTO")
    
#头文件路径
include_directories(
  "${PROJECT_SOURCE_DIR}/librtmp"
  )

link_directories("${PROJECT_SOURCE_DIR}/libs" )
target_link_libraries(demo_push_h264
  "${PROJECT_SOURCE_DIR}/libs/librtmp.a"
   )    
