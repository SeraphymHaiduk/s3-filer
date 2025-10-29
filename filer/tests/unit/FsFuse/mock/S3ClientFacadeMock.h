#include <gmock/gmock.h>
#include <S3ClientFacadeBase.h>

class S3ClientFacadeMock: public S3Filer::S3ClientFacadeBase {
public:
    S3ClientFacadeMock() = default;
    ~S3ClientFacadeMock() = default;

    MOCK_METHOD(const Aws::S3::S3Client&, getClient, (), (override));

    MOCK_METHOD(int, createFile, (const std::string& bucket, const std::string& filePath), (override));
    MOCK_METHOD(int, createDir, (const std::string& bucket, const std::string& filePath), (override));
    MOCK_METHOD(int, uploadFile, (const std::string& bucket, const std::string& filePath, const std::string& content), (override));
    MOCK_METHOD(int, deleteFile, (const std::string& bucket, const std::string& filePath), (override));
    MOCK_METHOD(int, deleteDir, (const std::string& bucket, const std::string& dirPath), (override));
    MOCK_METHOD(int, readFile, (const std::string &bucket, const std::string &path, char *buf, size_t size, off_t offset), (override));
    MOCK_METHOD(int, renameFolderRecursively, (const std::string &bucket, const std::string &old_path, const std::string &new_path), (override));
    MOCK_METHOD(int, renameSingleObject, (const std::string &bucket, const std::string &old_key, const std::string &new_key), (override));
    MOCK_METHOD(int, rename, (const std::string &bucket, const std::string &path, const std::string &new_path), (override));

    MOCK_METHOD(std::optional<struct stat>, getStat, (const std::string& bucket, const std::string& filePath), (override));
    MOCK_METHOD(std::optional<std::vector<S3Filer::ElementWithStat>>, readdir, (const std::string& bucket, const std::string& path), (override));

    MOCK_METHOD(bool, isConnectionEstablished, (), (override));
    MOCK_METHOD(bool, isDirExists, (const std::string& bucket, const std::string& path), (override));
};