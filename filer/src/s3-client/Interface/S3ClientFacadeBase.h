
#pragma once

#include <string>
#include <vector>
#include <optional>
#include <sys/stat.h>

namespace Aws {
    namespace S3 {
        class S3Client;
    }
}

namespace S3Filer {

    struct ElementWithStat {
        std::string name;
        struct stat stat;
    };

    class S3ClientFacadeBase {
        private:
        public:
            virtual bool isConnectionEstablished() = 0;
            virtual const Aws::S3::S3Client& getClient() = 0;

            virtual int createFile(const std::string& bucket, const std::string& filePath) = 0;
            virtual int createDir(const std::string& bucket, const std::string& filePath) = 0;
            virtual int uploadFile(const std::string& bucket, const std::string& filePath, const std::string& content) = 0;
            virtual int deleteFile(const std::string& bucket, const std::string& filePath) = 0;
            virtual int deleteDir(const std::string& bucket, const std::string& dirPath) = 0;
            virtual std::optional<struct stat> getStat(const std::string& bucket, const std::string& filePath) = 0;
            virtual std::optional<std::vector<ElementWithStat>> readdir(const std::string& bucket, const std::string& path) = 0;
            virtual int readFile(const std::string &bucket, const std::string &path, char *buf, size_t size, off_t offset) = 0;
            virtual int renameFolderRecursively(const std::string &bucket, const std::string &old_path, const std::string &new_path) = 0;
            virtual int renameSingleObject(const std::string &bucket, const std::string &old_key, const std::string &new_key) = 0;
            virtual int rename(const std::string &bucket, const std::string &path, const std::string &new_path) = 0;

            virtual bool isDirExists(const std::string& bucket, const std::string& path) = 0;
        };
}