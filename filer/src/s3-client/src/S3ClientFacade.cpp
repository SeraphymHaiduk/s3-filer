
#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <aws/s3/model/ListObjectsV2Request.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/CopyObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <optional>

#include <fstream>
#include <sstream>
#include "S3ClientFacade.h"
#include <filesystem>

using namespace S3Filer;

/**
 * Ensures that path will have slash at the end if don't have yet
 */
std::string S3Filer::ensureEndSlash(const std::string& path) 
{
    // path should end with '/'
    return path.back() == '/' ? path : (path + "/"); 
}

/**
 * Ensures that path won't have slash at the end
 */
std::string S3Filer::ensureNoEndSlash(const std::string& path) 
{
    if (path.back() == '/') {
        std::string newString = path;
        newString.pop_back();
        return newString;
    } else {
        return path;
    }
}

/**
 * Modifies path so it will have '/' in the beginning if don't have yet
 */
std::string S3Filer::ensureStartSlash(const std::string& path)
{
    return path.front() == '/' ? path : ('/' + path);
}

/**
 * Modifies path so it won't have '/' in the beginning   
 */
std::string S3Filer::ensureNoStartSlash(const std::string& path) {
    // s3 prefix shouldn't contain '/' as a first symbol.
    return path.length() > 0 && path.front() == '/' ? path.substr(1) : path;
}

uint64_t S3Filer::makeInoFromPath(const std::string& path) {
    return static_cast<uint64_t>(std::hash<std::string>{}(path));
}

S3ClientFacade::S3ClientFacade() 
: m_awsClientConfig( createAwsClientConfig() ),
  m_awsCredentials( new Aws::Auth::AWSCredentials("admin", "adminadmin") ),
  m_s3Client( new Aws::S3::S3Client(*m_awsCredentials, 
                                    *m_awsClientConfig,
                                    Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false))
{}

std::unique_ptr<Aws::Client::ClientConfiguration> S3ClientFacade::createAwsClientConfig()
{
    std::unique_ptr<Aws::Client::ClientConfiguration> config = std::make_unique<Aws::Client::ClientConfiguration>();
    config->endpointOverride = "127.0.0.1:9000";
    config->scheme = Aws::Http::Scheme::HTTP;
    return config;
}

bool S3ClientFacade::isConnectionEstablished() {
    Aws::S3::Model::ListBucketsRequest request;
    
    auto outcome = m_s3Client->ListBuckets(request);
    
    if (outcome.IsSuccess()) {
        std::cout << "S3 connection established to url: " << m_awsClientConfig->endpointOverride << std::endl;
        return true;
    } else {
        std::cerr << "isConnectionEstablished error: " << outcome.GetError().GetMessage() << std::endl;
        return false;
    }
}

const Aws::S3::S3Client& S3ClientFacade::getClient()
{
    return *m_s3Client;
}

int S3ClientFacade::createFile(const std::string& bucket, const std::string& filePath) {
    std::string emptyBody;
    return uploadFile(bucket, ensureNoStartSlash(ensureNoEndSlash(filePath)), emptyBody);
}

int S3ClientFacade::createDir(const std::string& bucket, const std::string& path) {
    std::string emptyBody;
    return uploadFile(bucket, ensureNoStartSlash(ensureEndSlash(path)), emptyBody);
}

/**
 * Reminder:
 * key for file shouldn't contain '/' at the beginning as all keys in S3 requests.
 * key for file MAY contain '/' at the end only if it's an imitation of the directory. If you wan't to upload file - remove end slash
 * This function will take care about it anyway, but use this description in case of any future changes.
 */
int S3ClientFacade::uploadFile(const std::string& bucket, const std::string& filePath, const std::string& content)
{
    std::cout << "S3ClientFacade::uploadFile bucket: " << bucket << " filePath: " << filePath << std::endl;
    Aws::S3::Model::PutObjectRequest request;
    request.SetBucket(bucket);
    request.SetKey(ensureNoStartSlash(filePath));

    auto input_data = Aws::MakeShared<Aws::StringStream>("UploadTag", content);
    request.SetBody(input_data);

    auto outcome = m_s3Client->PutObject(request);
    if (!outcome.IsSuccess()) {
        std::cerr << "Error: " << outcome.GetError().GetMessage() << "\n";
        return -EIO;
    }
    return 0;
}

int S3ClientFacade::deleteFile(const std::string& bucket, const std::string& filePath)
{
    Aws::S3::Model::DeleteObjectRequest request;
    request.SetBucket(bucket);
    request.SetKey(ensureNoStartSlash(ensureNoEndSlash(filePath)));
    Aws::S3::Model::DeleteObjectOutcome outcome = m_s3Client->DeleteObject(request);
    if (!outcome.IsSuccess())
        return -EIO;
    return 0;
}

int S3ClientFacade::deleteDir(const std::string& bucket, const std::string& dirPath)
{
    Aws::S3::Model::DeleteObjectRequest request;
    request.SetBucket(bucket);
    request.SetKey(ensureNoStartSlash(ensureEndSlash(dirPath)));
    Aws::S3::Model::DeleteObjectOutcome outcome = m_s3Client->DeleteObject(request);
    if (!outcome.IsSuccess())
        return -EIO;
    return 0;
}


/**
 * Reminder:
 * key for directory shouldn't contain '/' at the beginning as all keys in S3 requests.
 * key for directory should contain '/' at the end.
 * This function will take care about it anyway, but use this description in case of any future changes.
 */
bool S3ClientFacade::isDirExists(const std::string& bucket, const std::string& path) {
    Aws::S3::Model::HeadObjectRequest request;
    Aws::S3::Model::HeadObjectOutcome outcome;

    std::string key = ensureNoStartSlash(ensureEndSlash(path));

    request.SetBucket(bucket);
    request.SetKey(key);
    outcome = m_s3Client->HeadObject(request);

    std::cout << "S3ClientFacade::isDirExists dir imitation file with a key: " << key << " exists: " << outcome.IsSuccess() << std::endl;
    if (outcome.IsSuccess()) {
        return true;
    }

    Aws::S3::Model::ListObjectsV2Request list_request;
    list_request.SetBucket(bucket);
    list_request.SetPrefix(key);
    list_request.SetMaxKeys(1);

    Aws::S3::Model::ListObjectsV2Outcome list_request_outcome = m_s3Client->ListObjectsV2(list_request);
    if (list_request_outcome.IsSuccess()) {
        Aws::S3::Model::ListObjectsV2Result result = list_request_outcome.GetResult();
        std::cout << "S3ClientFacade::isDirExists outcome success. contents.empty(): "
            << (result.GetContents().empty() ? "true" : "false")
            << " commonprefixes.empty(): "
            << (result.GetCommonPrefixes().empty() ? "true" : "false")
            << std::endl;

        if (!(result.GetContents().empty() && result.GetCommonPrefixes().empty())) {
            return true;
        }
    }
    return false;
}

Aws::S3::Model::HeadObjectOutcome S3ClientFacade::getObject(const std::string& bucket, const std::string& key)
{
    Aws::S3::Model::HeadObjectRequest request;
    Aws::S3::Model::HeadObjectOutcome outcome;

    request.SetBucket(bucket);
    request.SetKey(ensureNoStartSlash(ensureNoStartSlash(ensureNoEndSlash(key))));
    return m_s3Client->HeadObject(request);
}


/**
 * Reminder:
 * key for directory OR file shouldn't contain '/' at the beginning as all keys in S3 requests.
 * key for file shouldn't contain '/' at the end.
 * This function will take care about it anyway, but use this description in case of any future changes.
 */
std::optional<struct stat> S3ClientFacade::getStat(const std::string& bucket, const std::string& path)
{
    struct stat stat_data;
    
    if (path == "/" || isDirExists(bucket, path)) {
        stat_data.st_size = 0;
        stat_data.st_mode = S_IFDIR | 0755;
        stat_data.st_nlink = 2;
        stat_data.st_uid = getuid();
        stat_data.st_gid = getgid();
        stat_data.st_ino = makeInoFromPath(ensureNoStartSlash(ensureNoEndSlash(path)));
        stat_data.st_mtime = 1.0;
        stat_data.st_ctime = 1.0;
        stat_data.st_atime = time(nullptr);
        return stat_data;
    }
    

    Aws::S3::Model::HeadObjectOutcome outcome = getObject(bucket, path);
    if (outcome.IsSuccess()) {
        Aws::S3::Model::HeadObjectResult result = outcome.GetResult();
        stat_data.st_size = result.GetContentLength();
        stat_data.st_mode = S_IFREG | 0664;
        stat_data.st_nlink = 1;
        stat_data.st_uid = getuid();
        stat_data.st_gid = getgid();
        stat_data.st_ino = makeInoFromPath(ensureNoStartSlash(ensureNoEndSlash(path)));
        stat_data.st_mtime = 1.0;
        stat_data.st_ctime = 1.0;
        stat_data.st_atime = time(nullptr);
        return stat_data;
    } 

    std::cerr << "S3ClientFacade::getStat: no object or folder found for bucket: " << bucket << " key: " << path << std::endl;
    return std::nullopt;
}

/**
 * Reminder:
 * key for directory shouldn't contain '/' at the beginning as all keys in S3 requests.
 * key for directory should contain '/' at the end.
 * This function will take care about it anyway, but use this description in case of any future changes.
 */
std::optional<std::vector<ElementWithStat>> S3ClientFacade::readdir(const std::string& bucket, const std::string& path)
{
    Aws::S3::Model::ListObjectsV2Request list_request;
    list_request.SetBucket(bucket);
    std::string dir_s3_prefix = ensureNoStartSlash(ensureEndSlash(path));
    list_request.SetPrefix(dir_s3_prefix);
    list_request.SetDelimiter("/");

    Aws::S3::Model::ListObjectsV2Outcome list_request_outcome = m_s3Client->ListObjectsV2(list_request);

    if (list_request_outcome.IsSuccess()) {
        std::vector<ElementWithStat> paths;
        Aws::S3::Model::ListObjectsV2Result result = list_request_outcome.GetResult();
        
        std::cout << "S3ClientFacade::readdir outcome success. contents.empty(): "
            << (result.GetContents().empty() ? "true" : "false")
            << " commonprefixes.empty(): "
            << (result.GetCommonPrefixes().empty() ? "true" : "false")
            << std::endl;

        if (!(result.GetContents().empty() && result.GetCommonPrefixes().empty())) {
            for (const auto &prefix: result.GetCommonPrefixes()) {
                std::cout << "S3ClientFacade::readdir: " << path << " common prefix: " << prefix.GetPrefix().c_str() << std::endl;
                ElementWithStat element;
                element.name = std::filesystem::path(ensureNoEndSlash( prefix.GetPrefix().c_str() )).filename().string();
                element.stat.st_mode = S_IFDIR | 0755;
                element.stat.st_size = 0;
                element.stat.st_nlink = 2;
                element.stat.st_uid = getuid();
                element.stat.st_gid = getgid();
                element.stat.st_ino = makeInoFromPath(ensureNoStartSlash(ensureNoEndSlash(prefix.GetPrefix().c_str())));
                element.stat.st_mtime = 1.0;
                element.stat.st_ctime = 1.0;
                element.stat.st_atime = time(nullptr);

                paths.push_back(element);
            }
            for (const auto &content: result.GetContents()) {
                std::cout << "S3ClientFacade::readdir: " << path << " content: " << content.GetKey().c_str() << std::endl;

                // Get rid of dir imitation file in result list
                if (dir_s3_prefix == content.GetKey().c_str())
                    continue;
                ElementWithStat element;
                element.name = std::filesystem::path(ensureNoEndSlash( content.GetKey().c_str() )).filename().string();
                element.stat.st_mode = S_IFREG | 0664;
                element.stat.st_size = content.GetSize();
                element.stat.st_nlink = 1;
                element.stat.st_uid = getuid();
                element.stat.st_gid = getgid();
                element.stat.st_ino = makeInoFromPath(ensureNoStartSlash(ensureNoEndSlash(content.GetKey().c_str())));
                element.stat.st_mtime = 1.0;
                element.stat.st_ctime = 1.0;
                element.stat.st_atime = time(nullptr);
                paths.push_back(element);
            }

            std::cout << "S3ClientFacade::readdir result paths vector length: " << paths.size() << std::endl;
            return paths;
        }
    }

    std::cerr << "S3ClientFacade::readdir error: " << list_request_outcome.GetError() << std::endl;
    return std::nullopt;
}

int S3ClientFacade::readFile(const std::string &bucket, const std::string &path, char *buf, size_t size, off_t offset) {
    std::string key = ensureNoStartSlash(ensureNoEndSlash(path));
    
    // 2. Form GetObjectRequest
    Aws::S3::Model::GetObjectRequest request;
    request.SetBucket(bucket);
    request.SetKey(key);
    
    // 3. CRITICAL: Set the Range header for partial read
    long long end_byte = (long long)offset + size - 1;
    std::string range_header = "bytes=" + std::to_string(offset) + "-" + std::to_string(end_byte);
    request.SetRange(range_header);

    auto outcome = m_s3Client->GetObject(request);

    if (outcome.IsSuccess()) {
        // 4. Read the data stream
        auto& stream = outcome.GetResult().GetBody();
        // Read data directly into the FUSE buffer (buf)
        stream.read(buf, size);
        
        // 5. Return actual bytes read
        return stream.gcount(); 
    } else {
        // Handle error (e.g., return -EIO)
        return -EIO;
    }
}

int S3ClientFacade::renameSingleObject(const std::string &bucket, const std::string &old_key, const std::string &new_key) 
{
    std::cout << "S3ClientFacade::renameSingleObject bucket: " << bucket << " old_key: " << old_key << " new_key: " << new_key << std::endl;

    // copying
    Aws::S3::Model::CopyObjectRequest copyRequest;
    
    // source: /<bucket>/<old_key>
    std::string copySource = ensureStartSlash(ensureEndSlash(bucket)) + ensureNoStartSlash(old_key); 
    copyRequest.SetCopySource(copySource);

    // destination
    copyRequest.SetBucket(bucket);
    copyRequest.SetKey(ensureNoStartSlash(new_key));

    auto copyOutcome = m_s3Client->CopyObject(copyRequest);

    if (!copyOutcome.IsSuccess()) {
        std::cerr << "S3 RENAME: Failed to copy " << old_key << " to " << new_key << ": " 
                << copyOutcome.GetError().GetMessage() << std::endl;
        if (copyOutcome.GetError().GetErrorType() == Aws::S3::S3Errors::NO_SUCH_KEY) {
            return -ENOENT;
        }
        return -EIO;
    }


    // 2. УДАЛЕНИЕ
    Aws::S3::Model::DeleteObjectRequest deleteRequest;
    deleteRequest.SetBucket(bucket);
    deleteRequest.SetKey(old_key);

    auto deleteOutcome = m_s3Client->DeleteObject(deleteRequest);
    
    // Успешность операции rename определяется успешностью копирования.
    // Если удаление не удалось, мы логгируем ошибку, но возвращаем успех. 
    // В противном случае, у нас есть две копии (утечка памяти/трафика).
    if (!deleteOutcome.IsSuccess()) {
        std::cerr << "S3 RENAME: WARNING - Failed to delete source " << old_key << " after successful copy: " 
                  << deleteOutcome.GetError().GetMessage() << std::endl;
        // Возвращаем EIO, чтобы уведомить FUSE о частичном сбое (зависит от требований)
        // Для FUSE часто лучше вернуть 0, если файл доступен по новому пути.
        // Здесь мы возвращаем 0, но это место для тонкой настройки политики ошибок.
    }
    
    return 0;
}

int S3ClientFacade::renameFolderRecursively(const std::string &bucket, const std::string &old_path, const std::string &new_path)
{
    // Мы предполагаем, что эта функция вызывается только для папок, 
    // или вызывающий код уже определил, что это папка.
    // Если old_path была "my_dir", то old_key - "my_dir".
    
    std::string old_prefix = ensureNoStartSlash(ensureEndSlash(old_path));
    std::string new_prefix = ensureNoStartSlash(ensureEndSlash(new_path));
    
    Aws::S3::Model::ListObjectsV2Request request;
    request.SetBucket(bucket);
    request.SetPrefix(old_prefix);
    
    int overall_result = 0;
    
    // Переменная для хранения токена для следующего запроса
    Aws::String continuationToken;

    // Цикл для обработки пагинации
    for (int i = 0; overall_result == 0; ++i) { // Продолжаем, пока нет ошибок и есть страницы
        
        // 1. Установка токена для текущего запроса (для всех, кроме первого)
        if (i > 0) {
            if (continuationToken.empty()) {
                // Если нет токена, но цикл продолжается, значит, пагинация закончена.
                break;
            }
            request.SetContinuationToken(continuationToken);
        }

        // 2. Получение страницы объектов
        auto outcome = m_s3Client->ListObjectsV2(request);
        
        if (!outcome.IsSuccess()) {
            std::cerr << "S3 RENAME: Failed to list objects for folder " << old_prefix << std::endl;
            overall_result = -EIO;
            break;
        }

        const auto& result_list = outcome.GetResult();
        
        // 3. Обработка объектов на текущей странице
        for (const auto& object : result_list.GetContents()) {
            std::string current_old_key = object.GetKey();
            // Вычисляем новый ключ, заменяя старый префикс на новый
            std::string current_new_key = new_prefix + current_old_key.substr(old_prefix.length());
            
            // Вызываем вспомогательную функцию Copy + Delete
            int op_result = renameSingleObject(bucket, current_old_key, ensureEndSlash(current_new_key));
            
            if (op_result != 0) {
                // Если хоть один объект не удалось переместить, прерываем цикл и возвращаем ошибку
                overall_result = op_result;
                goto end_pagination_loop; // Имитация break для всего цикла ListObjects
            }
        }
        
        // 4. Проверка и обновление токена для следующей итерации
        if (result_list.GetIsTruncated()) {
            continuationToken = result_list.GetNextContinuationToken();
        } else {
            // Пагинация завершена (нет больше страниц)
            break; 
        }
    }

    end_pagination_loop:
        return overall_result;
}

int S3ClientFacade::rename(const std::string &bucket, const std::string &path, const std::string &new_path)
{
    std::cout << "S3ClientFacade::rename bucket: " << bucket << " path: " << path << " new_path: " << new_path << std::endl;
    // single file case
    int result = renameSingleObject(bucket, ensureNoEndSlash(path), ensureNoEndSlash(new_path));
    if (result != -ENOENT) 
        return result;
    std::cout << "S3ClientFacade::rename not a file. Trying to copy as a folder" << std::endl;

    return renameFolderRecursively(bucket, path, new_path);
}
