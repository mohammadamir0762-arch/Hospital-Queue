cmake_minimum_required(VERSION 3.14)
project(hospital_queue_web)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build output
add_executable(server src/main.cpp)

# Include header-only HTTP library
target_include_directories(server PRIVATE third_party)

# Linux/macOS need pthread for the server
if(UNIX)
  target_link_libraries(server pthread)
endif()

# Enable higher warnings (optional)
if (MSVC)
  target_compile_options(server PRIVATE /W4)
else()
  target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic)
endif()
