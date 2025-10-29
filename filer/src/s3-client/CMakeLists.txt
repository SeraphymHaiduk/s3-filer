
add_library(FilerS3ClientWrapper STATIC
    AwsSdkManager.cpp    
    FilerS3ClientWrapper.cpp
    AwsSdkManager.h
    FilerS3ClientWrapper.h   
)
target_include_directories(FilerS3ClientWrapper
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(FilerS3ClientWrapper PUBLIC aws-cpp-sdk-core aws-cpp-sdk-s3)
