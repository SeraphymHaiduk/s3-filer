#include "FsFuse.h"
#include "FilerS3ClientWrapper.h"
#include "ContextManager.h"

using namespace S3Filer;

std::string bucket_default = "development";

using FilesType = std::vector<std::string>;

int s3filer_getattr(const char *path, struct stat *stbuf, fuse_file_info *fi)
{
    (void) fi;

    std::optional<struct stat> result = FilerS3ClientWrapper::instance().getStat(bucket_default, path);
    std::cout << "s3filer_getattr result.hasvalue(): " << (result.has_value() ? "true" : "false") << std::endl;
    if (result.has_value()) {
        std::cout << "s3filer_getattr result.value().st_mode: " << result.value().st_mode << std::endl;
        std::cout << "s3filer_getattr result.value().st_size: " << result.value().st_size << std::endl;
        std::cout << "s3filer_getattr result.value().st_nlink: " << result.value().st_nlink << std::endl;

        stbuf->st_mode = result.value().st_mode;
        stbuf->st_size = result.value().st_size;
        stbuf->st_nlink = result.value().st_nlink;
        stbuf->st_uid = result.value().st_uid;
        stbuf->st_gid = result.value().st_uid;
        stbuf->st_ino = result.value().st_uid;
        stbuf->st_mtime = result.value().st_mtime;
        stbuf->st_atime = result.value().st_atime;
        stbuf->st_ctime = result.value().st_ctime;
        return 0;
    }
    return -ENOENT;
}

int s3Filer_mkdir(const char *path, mode_t mode)
{
    if (FilerS3ClientWrapper::instance().createDir(bucket_default, path) == 0)
        return 0;
    return -EIO;
}

int s3filer_unlink (const char *path)
{
    std::optional<struct stat> result = FilerS3ClientWrapper::instance().getStat(bucket_default, path);

    std::cerr << "s3filer_getattr result.hasvalue(): " << (result.has_value() ? "true" : "false") << std::endl;
    if (!result.has_value())
        return -ENOENT;

    if (S_ISDIR(result.value().st_mode)) {
        // Cannot open dir, because it's not a file
        return -EISDIR;
    }

    return FilerS3ClientWrapper::instance().deleteFile(bucket_default, path);
}

int s3Filer_rmdir(const char *path)
{
    std::optional<struct stat> result = FilerS3ClientWrapper::instance().getStat(bucket_default, path);

    std::cerr << "s3filer_getattr result.hasvalue(): " << (result.has_value() ? "true" : "false") << std::endl;
    if (!result.has_value())
        return -ENOENT;

    if (S_ISREG(result.value().st_mode)) {
        // Cannot open dir, because it's not a file
        return -ENOTDIR;
    }

    std::optional<std::vector<ElementWithStat>> readdir_result = FilerS3ClientWrapper::instance().readdir(bucket_default, path);
    if (!readdir_result.has_value())
        return -EIO;

    if (readdir_result.value().size() > 0)
        return -ENOTEMPTY;

    return FilerS3ClientWrapper::instance().deleteDir(bucket_default, path);
}

int s3filer_rename (const char *path, const char *new_path, unsigned int flags) {
    return FilerS3ClientWrapper::instance().rename(bucket_default, path, new_path);
}

int s3filer_opendir(const char *path, fuse_file_info *fi) {
    if (FilerS3ClientWrapper::instance().isDirExist(bucket_default, path))
        return 0;
    
    return -ENOENT;
}

int s3filer_readdir (const char *path, void *buf, fuse_fill_dir_t filler,
            off_t offset, fuse_file_info *fi,
            enum fuse_readdir_flags flags)
{
    std::cout << "s3filer_readdir path: " << path << " offset: " << offset << std::endl; 

    if (offset == 0) {
        std::cout << "s3filer_readdir fill . with offset: " << 1 << std::endl;
        if (filler(buf, ".", NULL, 1, fuse_fill_dir_flags(0))) return 0;
    }
    if (offset <= 1) {
        std::cout << "s3filer_readdir fill .. with offset: " << 2 << std::endl;
        if (filler(buf, "..", NULL, 2, fuse_fill_dir_flags(0))) return 0;
    }

    std::optional<std::vector<ElementWithStat>> result = 
        FilerS3ClientWrapper::instance().readdir(bucket_default, path);

    std::cerr << "s3filer_opendir result.hasvalue(): " << (result.has_value() ? "true" : "false") <<std::endl;

    off_t current_offset = 3; // beginning of real files inside of the folder 

    if (result.has_value()) {
        // Использование ссылки на значение более эффективно, если readdir не создает новую копию.
        // Если FilerS3ClientWrapper::readdir возвращает временный объект, 
        // используйте std::vector<ElementWithStat> elements = result.value();
        std::vector<ElementWithStat>& elements = result.value(); 
        if (offset >= elements.size()) {
            std::cout << "s3filer_readdir: offset " << offset << " >= total " << elements.size() << ", returning EOF" << std::endl;
            return 0;
        }

        std::cout << "s3filer_opendir elements.size(): " << elements.size() << std::endl;
        for (ElementWithStat &element: elements) {
            
            if (current_offset <= offset) {
                current_offset++;
                continue;
            }

            // --- Подготовка имени ---
            if (!element.name.empty() && element.name.back() == '/') {
                element.name.pop_back();
            }
            
            std::cout << "s3filer_readdir fill " << element.name << " with offset: " << current_offset << std::endl;
            if (filler(buf, element.name.c_str(), &element.stat, current_offset, FUSE_FILL_DIR_PLUS)) {
                // Если filler вернул ненулевое значение, буфер полон.
                return 0; 
            }
            current_offset++;
        }
        // std::cout << "s3filer_readdir signaling EOF" << std::endl;
        // filler(buf, NULL, NULL, 0, fuse_fill_dir_flags(0));

        return 0;
    }
    return -ENOENT; 
}

int s3filer_releasedir (const char *, fuse_file_info *fi) {
    return 0;
}

int s3filer_chmod(const char *path, mode_t mode, fuse_file_info *fi) {
    (void) path; (void) mode; (void) fi;
    return 0;
}

int s3filer_chown(const char *path, uid_t uid, gid_t gid, fuse_file_info *fi) {
    (void) path; (void) uid; (void) gid; (void) fi;
    return 0;
}

int s3filer_truncate(const char *path, off_t size, fuse_file_info *fi) {
    (void) path; (void) size; (void) fi;
    std::optional<struct stat> result = FilerS3ClientWrapper::instance().getStat(bucket_default, path);

    std::cerr << "s3filer_getattr result.hasvalue(): " << (result.has_value() ? "true" : "false") << std::endl;
    if (!result.has_value())
        return -ENOENT;

    if (S_ISDIR(result.value().st_mode)) {
        // Cannot open dir, because it's not a file
        return -EISDIR;
    }

    return FilerS3ClientWrapper::instance().uploadFile(bucket_default, path, "");
}


/**
 * TODO: this function should also check if it's allowed to open a file in a mode requested by user in: fi->flags (contais requested modes) 
 * also you can specify session descriptor: fi->fh
 * and tune cache settings by: fi->direct_io or fi->keep_cache
 * 
 * TODO: check usage of fi->fh for this function
 * 
 * TODO: return -EACCES if user not privileged for action
 */
int s3filer_open(const char *path, fuse_file_info *fi) {
    std::optional<struct stat> result = FilerS3ClientWrapper::instance().getStat(bucket_default, path);

    std::cerr << "s3filer_getattr result.hasvalue(): " << (result.has_value() ? "true" : "false") << std::endl;

    if ((fi->flags & O_CREAT) && !result.has_value()) {
        bool creationResult = FilerS3ClientWrapper::instance().createFile(bucket_default, path);
        if (creationResult != 0) 
            return -EIO;
    } else if (!result.has_value())
    {
        return -ENOENT;
    }
    
    if (S_ISDIR(result.value().st_mode)) {
        // Cannot open dir, because it's not a file
        return -EISDIR;
    }


    if (!(fi->flags & (O_WRONLY | O_RDWR))) {
        // File opened for reading: just return 0
        return 0;
    }

    uint64_t newFileHandler = ContextManager::instance().createMultipartUpload(bucket_default, path);
    if (!newFileHandler)
        return -EIO;

    fi->fh = newFileHandler;

    return 0;
}

/**
 * TODO: check usage of fi->fh for this function
 */
int s3filer_read(const char *path, char *buf, size_t size, off_t offset,
            fuse_file_info *fi) 
{
    return FilerS3ClientWrapper::instance().readFile(bucket_default, path, buf, size, offset);
}    

/**
 * TODO: check usage of fi->fh for this function
 */
int s3filer_write(const char *path, const char *buf, size_t size, off_t offset,
            struct fuse_file_info *fi)
{
    return ContextManager::instance().uploadPart(fi->fh, buf, size);
}

/**
 * TODO: check usage of fi->fh for this function
 */
int s3filer_release(const char *path, fuse_file_info *fi) {
    if (! (fi->flags & (O_WRONLY | O_RDWR | O_CREAT))) {
        // Remove multipart upload context if exists, because file was opened only for reading
        ContextManager::instance().cancelMultipartUpload(fi->fh);
        return 0;
    }

    return ContextManager::instance().completeMultipartUpload(fi->fh);
}

/**
 * TODO: check usage of fi->fh for this function
 */
int s3filer_create(const char *path, mode_t mode, fuse_file_info *fi) {
    if (FilerS3ClientWrapper::instance().createFile(bucket_default, path) != 0) {
        return -EIO;
    }

    if (fi->flags & (O_WRONLY | O_RDWR)) {
        int fileHandler = ContextManager::instance().createMultipartUpload(bucket_default, path);
        if (!fileHandler)
            return -EIO;

        fi->fh = fileHandler;
    }
    return 0;
}

int s3filer_utimens(const char *, const struct timespec tv[2],
            struct fuse_file_info *fi)
{
    return 0;
}

const struct fuse_operations s3filer_operations = {
    .getattr    = s3filer_getattr,
    .readlink   = NULL,
    .mknod      = NULL,
    .mkdir      = s3Filer_mkdir,
    .unlink     = s3filer_unlink,
    .rmdir      = s3Filer_rmdir,
    .symlink    = NULL,
    .rename     = s3filer_rename,
    .link       = NULL,
    .chmod      = s3filer_chmod,
    .chown      = s3filer_chown,
    .truncate   = s3filer_truncate,
    .open       = s3filer_open,
    .read       = s3filer_read,
    .write      = s3filer_write,
    .release    = s3filer_release,
    .opendir    = s3filer_opendir,
    .readdir    = s3filer_readdir,
    .releasedir = s3filer_releasedir,
    .create     = s3filer_create,
    .utimens    = s3filer_utimens,
};

int S3Filer::initS3Fuse(int argc, char *argv[]) {
    return fuse_main(argc, argv, &s3filer_operations, NULL);
}
