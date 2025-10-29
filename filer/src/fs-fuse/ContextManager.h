
#include <string>
#include <map>
#include <memory>

namespace S3Filer {
    struct UploadContext {
        std::string s3Key;
        std::string uploadId;
        std::string bucketName;
        int nextPartNumber = 1;

        std::map<int, std::string> completedParts; // map<PartNumber, ETag>
    };

    class ContextManager {
    private:
        ContextManager() = default;
        ~ContextManager() = default;

        std::shared_ptr<UploadContext> getUploadContext(uint64_t fh);

    public:
        ContextManager(const ContextManager&) = delete;
        ContextManager& operator=(const ContextManager&) = delete;

        static ContextManager& instance();
        
        /**
         * @brief Creates unique file handler.
         * @return Unique file handler.
         */
        uint64_t getNewFileHandler();

        /**
         * @brief Creates multi-part upload context.
         * * @param bucket S3 bucket name.
         * * @param key S3 object key.
         * @return Unique file handler (which also serves as a unique upload context identifier).
         * in case of success, or 0 in case of error.
         */
        uint64_t createMultipartUpload(std::string bucket, std::string key);

        /**
         * @brief removes upload context if exists
         */
        void cancelMultipartUpload(uint64_t fileHandler);

        /**
         * @brief Uploads part of the multipart upload for file
         * * @param fileHandler Unique upload context identifier.
         * * @param buf Buffer with a part data.
         * * @param size Buffer size.
         * @return Amount of written bytes in case of success, or negative value in case of error.
         */
        int uploadPart(uint64_t fileHandler, const char *buf, off_t size);

        /**
         * @brief Attempts to finishing the process of multi-part upload and clears upload context in any case.
         * 
         * NOTE: File handler stays unaffcted. Only upload context clears.
         * * @param fileHandler Unique file handler (which also serves as a unique upload context identifier).
         * @return 0 in case of success, or negative value in case of error.
         */
        int completeMultipartUpload(uint64_t fileHandler);

    private:
        std::map<uint64_t, std::shared_ptr<UploadContext>> m_uploadContexts;
        uint64_t m_nextFileHandler = 2;
        std::mutex m_mutex;
    };
};