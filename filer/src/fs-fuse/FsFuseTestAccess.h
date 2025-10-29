#define FUSE_USE_VERSION 30
#include "fuse3/fuse.h"

extern const struct fuse_operations& __test_get_s3filer_operations();

extern int __test_s3filer_getattr(const char *path, struct stat *stbuf, fuse_file_info *fi);

extern int __test_s3Filer_mkdir(const char *path, mode_t mode);

extern int __test_s3filer_unlink (const char *path);

extern int __test_s3Filer_rmdir(const char *path);

extern int __test_s3filer_rename (const char *path, const char *new_path, unsigned int flags);

extern int __test_s3filer_opendir(const char *path, fuse_file_info *fi);

extern int __test_s3filer_readdir (const char *path, void *buf, fuse_fill_dir_t filler,
                off_t offset, fuse_file_info *fi,
                enum fuse_readdir_flags flags);

extern int __test_s3filer_releasedir (const char *, fuse_file_info *fi);

extern int __test_s3filer_chmod(const char *path, mode_t mode, fuse_file_info *fi);

extern int __test_s3filer_chown(const char *path, uid_t uid, gid_t gid, fuse_file_info *fi);

extern int __test_s3filer_truncate(const char *path, off_t size, fuse_file_info *fi);

extern int __test_s3filer_open(const char *path, fuse_file_info *fi);

extern int __test_s3filer_read(const char *path, char *buf, size_t size, off_t offset,
                fuse_file_info *fi);

extern int __test_s3filer_write(const char *path, const char *buf, size_t size, off_t offset,
                struct fuse_file_info *fi);

extern int __test_s3filer_release(const char *path, fuse_file_info *fi);

extern int __test_s3filer_create(const char *path, mode_t mode, fuse_file_info *fi);

extern int __test_s3filer_utimens(const char *, const struct timespec tv[2],
                struct fuse_file_info *fi);
