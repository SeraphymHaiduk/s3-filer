
add_library(S3ClientFacade STATIC
    src/AwsSdkManager.cpp    
    src/S3ClientFacade.cpp
)

target_include_directories(S3ClientFacade
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Interface
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(S3ClientFacade PUBLIC aws-cpp-sdk-core aws-cpp-sdk-s3)
