add_executable(Test_FS_Fuse
    core/main.cpp
    core/FsFuseTest.cpp
)

target_include_directories(Test_FS_Fuse
    PRIVATE
    core/
    mock/
)

target_compile_definitions(Test_FS_Fuse PRIVATE 
    UNIT_TESTING
)

get_target_property(FS_FUSE_SOURCE_DIR FsFuse SOURCE_DIR)

target_include_directories(Test_FS_Fuse PRIVATE
    ${FS_FUSE_SOURCE_DIR}
)

target_link_libraries(Test_FS_Fuse
    gtest
    gmock
    FsFuse
)

add_test(
    NAME Unit.FsFuse
    COMMAND Test_FS_Fuse
)

# gtest_discover_tests(Test_FS_Fuse)