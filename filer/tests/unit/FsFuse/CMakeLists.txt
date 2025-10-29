add_executable(Test_FS_Fuse
    main.cpp
    FsFuseTest.cpp
    FsFuseTest.h
)

target_link_libraries(Test_FS_Fuse
    gtest
    FsFuse
)

add_test(
    NAME Unit.FsFuse
    COMMAND Test_FS_Fuse
)

# gtest_discover_tests(Test_FS_Fuse)