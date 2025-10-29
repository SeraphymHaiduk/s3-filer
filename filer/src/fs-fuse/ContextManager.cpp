#include "ContextManager.h"
#include "FilerS3ClientWrapper.h"
#include <aws/s3/model/CreateMultipartUploadRequest.h>
#include <aws/s3/model/UploadPartRequest.h>
#include <aws/s3/model/CompleteMultipartUploadRequest.h>

using namespace S3Filer;

ContextManager& ContextManager::instance() {
    static ContextManager manager;
    return manager;
}

std::shared_ptr<UploadContext> ContextManager::getUploadContext(uint64_t fileHandler)
{
    if (m_uploadContexts.count(fileHandler)) {
        return m_uploadContexts.at(fileHandler);
    }
    return nullptr;
}

uint64_t ContextManager::getNewFileHandler()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_nextFileHandler++;
}

uint64_t ContextManager::createMultipartUpload(std::string bucket, std::string key)
{
    std::cout << "ContextManager::createMultipartUpload bucket: " << bucket << " key: " << key << std::endl;
    std::shared_ptr<UploadContext> ctx = std::make_shared<UploadContext>();
    ctx->s3Key = ensureNoStartSlash(std::string(key));
    ctx->bucketName = bucket; 

    Aws::S3::Model::CreateMultipartUploadRequest request;
    request.SetBucket(ctx->bucketName.c_str());
    request.SetKey(ctx->s3Key.c_str());

    auto outcome = FilerS3ClientWrapper::instance().getClient().CreateMultipartUpload(request);

    if (outcome.IsSuccess()) {
        ctx->uploadId = outcome.GetResult().GetUploadId();
        
        
        uint64_t current_fh = getNewFileHandler();
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_uploadContexts[current_fh] = ctx;
        }
        
        std::cout << "ContextManager::createMultipartUpload Multipart upload started for: " << ctx->s3Key << " ID: " << ctx->uploadId << std::endl;
        return current_fh;
    } else {
        std::cerr << "ContextManager::createMultipartUpload Error creating MPU: " << outcome.GetError().GetMessage() << std::endl;
        return 0;
    }
}

void ContextManager::cancelMultipartUpload(uint64_t fileHandler)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_uploadContexts.erase(fileHandler);
}

/**
 * TODO: разобраться с поддержкой lseek
 */
int ContextManager::uploadPart(uint64_t fileHandler, const char *buf, off_t size)
{
    std::shared_ptr<UploadContext> ctx = getUploadContext(fileHandler);
    if (!ctx) {
        std::cout << "ContextManager::uploadPart Error: context not found. Return -EIO" << std::endl;
        return -EIO;
    }

    // FUSE может вызывать write с произвольным смещением. 
    // Для S3 MPU это сложно, но для простоты предполагается, что части идут последовательно.
    // Если нужна поддержка произвольного lseek, потребуется буферизация на диске.
    Aws::S3::Model::UploadPartRequest request;
    request.SetBucket(ctx->bucketName.c_str());
    request.SetKey(ctx->s3Key.c_str());
    request.SetUploadId(ctx->uploadId.c_str());
    request.SetPartNumber(ctx->nextPartNumber); // Увеличиваем номер части
    
    auto stream = Aws::MakeShared<Aws::StringStream>("UploadPartStream", std::string(buf, size));
    request.SetBody(stream);
    request.SetContentLength(size);

    auto outcome = FilerS3ClientWrapper::instance().getClient().UploadPart(request);

    if (outcome.IsSuccess()) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            // CRITICAL: save ETag for finishing step
            ctx->completedParts[ctx->nextPartNumber] = outcome.GetResult().GetETag();
            ctx->nextPartNumber++;
        }
        return size;
    } else {
        std::cerr << "ContextManager::uploadPart Error uploading part: " << outcome.GetError().GetMessage() << std::endl;
        return -EIO;
    }
}

int ContextManager::completeMultipartUpload(uint64_t fileHandler)
{

    // Получаем контекст для записи
    std::shared_ptr<UploadContext> ctx = getUploadContext(fileHandler);
    if (!ctx) {
        return 0; // Уже обработано или нет контекста
    }
    
    Aws::S3::Model::CompleteMultipartUploadRequest request;
    request.SetBucket(ctx->bucketName.c_str());
    request.SetKey(ctx->s3Key.c_str());
    request.SetUploadId(ctx->uploadId.c_str());

    Aws::S3::Model::CompletedMultipartUpload completedUpload;
    
    // Формируем список CompletedPart из сохраненных ETag
    for (const auto& pair : ctx->completedParts) {
        Aws::S3::Model::CompletedPart part;
        part.SetPartNumber(pair.first);
        part.SetETag(pair.second);
        completedUpload.AddParts(part);
    }
    request.SetMultipartUpload(completedUpload);

    auto outcome = FilerS3ClientWrapper::instance().getClient().CompleteMultipartUpload(request);

    // Clear context no matter what result is
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_uploadContexts.erase(fileHandler);
    }

    if (outcome.IsSuccess()) {
        std::cerr << "ContextManager::completeMultipartUpload Successfully completed MPU for: " << ctx->s3Key << std::endl;
        return 0;
    } else {
        std::cerr << "ContextManager::completeMultipartUpload Error completing MPU: " << outcome.GetError().GetMessage() << std::endl;
        // В случае сбоя, вы можете вызвать AbortMultipartUpload 
        // для очистки незавершенных частей.
        return -EIO;
    }
}
