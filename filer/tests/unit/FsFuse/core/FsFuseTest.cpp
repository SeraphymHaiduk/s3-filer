
#include "FsFuseTest.h"
#include "FsFuseTestAccess.h"
#include "FsFuse.h"
#include "S3ClientFacadeManager.h"

using namespace S3Filer;


FsFuseTest::FsFuseTest()
:m_clientFacadeMock(new S3ClientFacadeMock())
{
}

void FsFuseTest::SetUp()
{
    S3ClientFacadeManager::setS3ClientFacade(m_clientFacadeMock);
}


TEST_F(FsFuseTest, fuse_operaions)
{
    const fuse_operations &operations = __test_get_s3filer_operations();

    // Implemented functions
    ASSERT_NE(operations.getattr,   (void*)NULL);
    ASSERT_NE(operations.mkdir,     (void*)NULL);
    ASSERT_NE(operations.unlink,    (void*)NULL);
    ASSERT_NE(operations.rmdir,     (void*)NULL);
    ASSERT_NE(operations.rename,    (void*)NULL);
    ASSERT_NE(operations.chmod,     (void*)NULL);
    ASSERT_NE(operations.chown,     (void*)NULL);
    ASSERT_NE(operations.truncate,  (void*)NULL);
    ASSERT_NE(operations.open,      (void*)NULL);
    ASSERT_NE(operations.read,      (void*)NULL);   
    ASSERT_NE(operations.write,     (void*)NULL); 
    ASSERT_NE(operations.release,   (void*)NULL);  
    ASSERT_NE(operations.opendir,   (void*)NULL);  
    ASSERT_NE(operations.readdir,   (void*)NULL);
    ASSERT_NE(operations.releasedir,(void*)NULL);
    ASSERT_NE(operations.create,    (void*)NULL);
    ASSERT_NE(operations.utimens,   (void*)NULL);

    // Not implemented functions
    ASSERT_EQ(operations.readlink,  (void*)NULL);
    ASSERT_EQ(operations.mknod,     (void*)NULL);
    ASSERT_EQ(operations.symlink,   (void*)NULL);
    ASSERT_EQ(operations.link,      (void*)NULL);

}

TEST_F(FsFuseTest, getattr)
{
    EXPECT_CALL(*m_clientFacadeMock, getStat("development", "zxc"));

    struct stat stat;
    fuse_file_info fi;
    __test_s3filer_getattr("zxc", &stat, &fi);
}


