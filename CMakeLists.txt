cmake_minimum_required(VERSION 3.16)

project(vision_tracker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

## 给 clangd / VSCode IntelliSense 生成 compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(tracker
  src/main.cpp
  include/data_types.hpp
  include/test_frames.hpp
  src/TargetManager/TargetManager.cpp
  src/TargetManager/TrackingBase.cpp
  src/TargetManager/ArmorTarget.cpp
  src/TargetManager/SpinningTarget.cpp
)

target_include_directories(tracker PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(tracker PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/TargetManager")