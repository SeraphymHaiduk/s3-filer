
cmake_minimum_required(VERSION 3.22)

project(S3Filer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# AWS SDK
set(BUILD_ONLY "s3;core")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries for AWS SDK")
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

add_subdirectory(${PROJECT_SOURCE_DIR}/external/aws-sdk-cpp EXCLUDE_FROM_ALL)


add_subdirectory(src/fs-fuse)

add_subdirectory(src/s3-client)

add_executable(S3Filer
  src/main.cpp
)

set(BUILD_TESTS "ON" CACHE BOOL "Set to ON to enable the building of unit tests.")

target_link_libraries(S3Filer 
PRIVATE 
  FsFuse
)

if (BUILD_TESTS)
  message(STATUS "Adding tests")
  enable_testing()
  add_subdirectory(tests)
endif()


