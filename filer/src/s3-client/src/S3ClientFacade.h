
#pragma once
#include <aws/core/auth/AWSCredentials.h>
#include "S3ClientFacadeBase.h"
#include <aws/s3/S3Client.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <aws/core/Aws.h>

namespace Aws {
    namespace Client {
        class ClientConfiguration;
    }
    namespace Auth {
        class AWSCredentials;
    }
    namespace S3 {
        class S3Client;
    }
}

namespace S3Filer {
    std::string ensureEndSlash(const std::string& path);
    std::string ensureNoEndSlash(const std::string& path);
    std::string ensureStartSlash(const std::string& path);
    std::string ensureNoStartSlash(const std::string& path);
    
    uint64_t makeInoFromPath(const std::string& path);

    class S3ClientFacade : public S3ClientFacadeBase {
    private:
        static std::unique_ptr<Aws::Client::ClientConfiguration> createAwsClientConfig();
        Aws::S3::Model::HeadObjectOutcome getObject(const std::string& bucket, const std::string& key);

    public:
        S3ClientFacade();
        ~S3ClientFacade() = default;

        S3ClientFacade(const S3ClientFacade&) = delete;
        S3ClientFacade& operator=(const S3ClientFacade&) = delete;

        static S3ClientFacade& instance();
        bool isConnectionEstablished() override;
        const Aws::S3::S3Client& getClient() override;

        int createFile(const std::string& bucket, const std::string& filePath) override;
        int createDir(const std::string& bucket, const std::string& filePath) override;
        int uploadFile(const std::string& bucket, const std::string& filePath, const std::string& content) override;
        int deleteFile(const std::string& bucket, const std::string& filePath) override;
        int deleteDir(const std::string& bucket, const std::string& dirPath) override;
        std::optional<struct stat> getStat(const std::string& bucket, const std::string& filePath) override;
        std::optional<std::vector<ElementWithStat>> readdir(const std::string& bucket, const std::string& path) override;
        int readFile(const std::string &bucket, const std::string &path, char *buf, size_t size, off_t offset) override;
        int renameFolderRecursively(const std::string &bucket, const std::string &old_path, const std::string &new_path) override;
        int renameSingleObject(const std::string &bucket, const std::string &old_key, const std::string &new_key) override;
        int rename(const std::string &bucket, const std::string &path, const std::string &new_path) override;

        bool isDirExists(const std::string& bucket, const std::string& path) override;
    private:
        std::unique_ptr<Aws::Client::ClientConfiguration> m_awsClientConfig;
        std::unique_ptr<Aws::Auth::AWSCredentials> m_awsCredentials;
        std::unique_ptr<Aws::S3::S3Client> m_s3Client;
        std::unique_ptr<Aws::SDKOptions> m_sdkOptions;
    };
}
