
find_package(PkgConfig REQUIRED)
pkg_check_modules(FUSE REQUIRED fuse3) 

# Проверка, была ли найдена библиотека
if(NOT FUSE_FOUND)
    message(FATAL_ERROR "library libfuse was not found via pkg-config. Make sure that libfuse-dev was installed.")
endif()


add_library(FsFuse STATIC
    ContextManager.cpp
    FsFuse.cpp
    ContextManager.h
    FsFuse.h
    S3ClientFacadeManager.h
    S3ClientFacadeManager.cpp
)

if (BUILD_TESTS)
  target_compile_definitions(FsFuse PRIVATE 
    UNIT_TESTING
  )
endif()

target_include_directories(FsFuse
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${FUSE_INCLUDE_DIRS}
)

target_link_libraries(FsFuse 
    PRIVATE 
        ${FUSE_LIBRARIES}
    PUBLIC
        S3ClientFacade # TODO: use AwsSdkManager for initialization inside of the FsFuse and don't expose it
)