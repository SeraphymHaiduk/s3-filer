#include <gtest/gtest.h>
#include <../mock/S3ClientFacadeMock.h>

class FsFuseTest : public ::testing::Test {
public:
    FsFuseTest();
protected:
    void SetUp() override;
    void TearDown() override {}
public:
    std::shared_ptr<S3ClientFacadeMock> m_clientFacadeMock; 
};