#pragma once

#include <aws/s3/S3Client.h>
#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <sys/stat.h>
#include <optional>

namespace S3Filer {

    struct ElementWithStat {
        std::string name;
        struct stat stat;
    };

    std::string ensureEndSlash(const std::string& path);
    std::string ensureNoEndSlash(const std::string& path);
    std::string ensureStartSlash(const std::string& path);
    std::string ensureNoStartSlash(const std::string& path);
    
    uint64_t makeInoFromPath(const std::string& path);

    class FilerS3ClientWrapper {
    private:
        FilerS3ClientWrapper();
        ~FilerS3ClientWrapper() = default;
        
        static Aws::Client::ClientConfiguration createAwsClientConfig();

    public:
        FilerS3ClientWrapper(const FilerS3ClientWrapper&) = delete;
        FilerS3ClientWrapper& operator=(const FilerS3ClientWrapper&) = delete;

        static FilerS3ClientWrapper& instance();
        bool isConnectionEstablished();
        const Aws::S3::S3Client& getClient();

        int createFile(const std::string& bucket, const std::string& filePath);
        int createDir(const std::string& bucket, const std::string& filePath);
        int uploadFile(const std::string& bucket, const std::string& filePath, const std::string& content);
        int deleteFile(const std::string& bucket, const std::string& filePath);
        int deleteDir(const std::string& bucket, const std::string& dirPath);
        std::optional<struct stat> getStat(const std::string& bucket, const std::string& filePath);
        std::optional<std::vector<ElementWithStat>> readdir(const std::string& bucket, const std::string& path);
        int readFile(const std::string &bucket, const std::string &path, char *buf, size_t size, off_t offset);
        int renameFolderRecursively(const std::string &bucket, const std::string &old_path, const std::string &new_path);
        int renameSingleObject(const std::string &bucket, const std::string &old_key, const std::string &new_key);
        int rename(const std::string &bucket, const std::string &path, const std::string &new_path);

        bool isDirExist(const std::string& bucket, const std::string& path);
        Aws::S3::Model::HeadObjectOutcome getObject(const std::string& bucket, const std::string& key);

    private:
        Aws::Client::ClientConfiguration m_awsClientConfig;
        Aws::Auth::AWSCredentials m_awsCredentials;
        Aws::S3::S3Client m_s3Client;
        Aws::SDKOptions m_sdkOptions;
    };
}
