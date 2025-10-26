#define FUSE_USE_VERSION 30

#include <fuse3/fuse_opt.h>
#include <fuse3/fuse.h>
#include <cstring>
#include <stdio.h>

#include <string>
#include <vector>

#include <memory>
#include <algorithm>

std::string file_mock_content = R"(                                                    
                                                      ████                                
                                                      ██░░████                            
                                                      ██░░░░░░██                          
                                                      ██░░░░░░░░██                        
                                                      ██░░░░░░░░░░▓▓░░                    
                                                      ██  ░░░░░░░░░░▓▓░░                  
                                                      ██    ░░░░░░░░░░██                  
                                                      ██    ░░░░░░░░░░░░██                
                                                      ██      ░░░░░░░░░░░░██              
                                                      ██  ░░    ░░░░░░░░░░░░██            
                ░░██████████████████████        ██████████░░      ░░░░░░░░░░██            
                ░░██░░░░░░░░░░░░░░░░░░  ██    ██          ██            ░░░░░░██          
                ░░██  ░░░░░░░░░░░░░░      ████              ████              ██          
                ░░██  ░░░░░░░░░░░░        ██░░                ░░▓▓              ██        
                  ██  ░░░░░░░░░░        ▓▓░░                    ░░▓▓            ██        
                ░░██░░  ░░░░░░░░    ░░██                                        ██        
                ░░▓▓░░  ░░░░░░░░      ▓▓                                        ▓▓░░      
                    ██  ░░  ░░                                                    ██      
                    ██░░                                                          ██      
                    ██░░                                                          ██      
                    ██░░    ░░                                                      ██    
                      ██      ██                                                    ██    
                      ██░░    ██                                                    ██    
                      ██░░    ██                                                    ██    
                      ░░▒▒    ██                                                    ░░▒▒  
                        ▓▓    ██                                                      ██  
                        ▓▓    ██                                                      ██  
                        ▒▒    ██                                                      ██  
                        ░░██  ██                                                      ██  
                            ████                                                      ██  
                              ██                                ████                  ██  
                              ██                              ████                      ██
                              ██      ██████                ████        ████            ██
                              ██          ██████                      ██                ██
                              ██              ██                                        ██
                              ██  ██▓▓              ██                  ██            ██  
                              ██  ░░░░▓▓            ░░                  ░░▓▓▓▓      ▓▓░░  
                ░░████████████                                                    ██      
              ██▓▓          ██      ▓▓                                          ██        
          ▓▓▓▓              ██  ▓▓▓▓░░                                        ▓▓░░        
        ▓▓                  ░░██░░░░                                  ▓▓▓▓▓▓▓▓░░          
      ██                        ████            ████████████    ██████    ░░██            
    ██                              ██████    ██░░░░░░░░░░░░████░░░░      ░░██            
    ██                                ██  ████░░░░░░░░░░░░░░░░██░░░░      ░░██            
  ▓▓                                  ██  ██░░░░░░░░░░░░░░░░░░██░░░░  ▓▓██▓▓  ▓▓          
  ██                                  ██  ██░░░░░░░░░░░░░░░░░░░░▓▓▓▓▒▒░░░░░░  ██          
▓▓░░                                ▓▓██▒▒▒▒░░░░░░░░░░░░░░░░░░░░██░░░░        ░░▒▒        
██                                  ▓▓  ██░░░░░░░░░░██░░░░░░░░░░░░██            ██        
  ██                                ▓▓  ██░░░░░░░░██████░░░░░░░░░░██            ██        
    ██                              ▓▓    ██░░░░░░██████░░░░░░░░░░██            ██        
    ░░▓▓                            ▓▓    ██░░░░░░░░██░░░░░░░░░░▓▓░░            ░░██      
      ██                            ▓▓    ▒▒▓▓░░░░░░██░░░░░░░░░░██                ██      
        ██                          ▓▓        ██░░░░██░░░░░░░░██                  ██      
        ██                          ▓▓        ██░░░░██░░░░▓▓██                    ██      
          ▓▓                        ▓▓          ▓▓▓▓██▓▓▓▓                        ██      
          ██░░░░                    ▓▓                                            ██      
            ██░░                  ██                                              ██      
            ██░░░░                ██                                              ██      
              ██░░░░            ░░██                                            ██        
                ▓▓░░░░░░░░░░░░░░░░░░▓▓                                          ██        
                ░░▓▓░░░░░░░░░░░░░░░░▓▓                                        ▓▓░░        
                    ██░░░░░░░░░░░░░░▓▓                                    ████            
                      ██████████████  ██                          ████████                
                      ░░░░░░  ░░░░░░  ░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░                
                                        ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒                        
)";
typedef std::vector<std::string> FilesType;

FilesType files_mock {
    "HelloFile.txt"
};

std::unique_ptr<FilesType> dirFiles() {
    std::unique_ptr<std::vector<std::string>> content = std::make_unique<FilesType>(
        std::initializer_list<std::string>{
            ".",
            ".."
        }
    );

    content->insert(content->end(), files_mock.begin(), files_mock.end());
    return content;
}

namespace S3Filer {
    int s3filer_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
    {
        (void) fi;
        if (strcmp(path, "/") == 0) {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
        } else {
            FilesType::iterator it;
            if (strlen(path) > 1) {
                it = std::find(files_mock.begin(), files_mock.end(),std::string(path + 1));
            } else {
                it = files_mock.end();
            }
            
            if (it != files_mock.end()) {
                stbuf->st_mode = S_IFREG | 0755;    
                stbuf->st_size = file_mock_content.length();
                stbuf->st_nlink = 1;
            } else {
                return -ENOENT;
            }
            
        }
        return 0;
    }

    int s3filer_unlink (const char *path)
    {
        auto it = std::find(files_mock.begin(), files_mock.end(), std::string(path + 1));
        if (it == files_mock.end()) {
            return -ENOENT;
        }
        files_mock.erase(it, it+1);
        return 0;
    }

    int s3filer_rename (const char *path, const char *new_path, unsigned int flags) {
        auto it = std::find(files_mock.begin(), files_mock.end(), std::string(path + 1));
        if (it == files_mock.end()) {
            return -ENOENT;
        }

        *it = std::string(new_path + 1);
        return 0;
    }

    int s3filer_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi,
			 enum fuse_readdir_flags flags)
    {
        auto files = dirFiles();
        for (std::string& element: *files ) {
            filler(buf, element.c_str(), NULL, 0, FUSE_FILL_DIR_PLUS);
        }
        return 0;
    }

    int s3filer_chmod(const char *path, mode_t mode, struct fuse_file_info *fi) {
        (void) path; (void) mode; (void) fi;
        return 0; // Успех
    }

    int s3filer_chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi) {
        (void) path; (void) uid; (void) gid; (void) fi;
        return 0; // Успех
    }

    int s3filer_truncate(const char *path, off_t size, struct fuse_file_info *fi) {
        (void) path; (void) size; (void) fi;
        // Оставить заглушкой или реализовать логику сброса размера в вашем map!
        return 0; 
    }
    
    int s3filer_open(const char *path, struct fuse_file_info *fi) {
        auto it = std::find(files_mock.begin(), files_mock.end(), std::string(path + 1));
        if (it == files_mock.end()) {
            return -ENOENT;
        }
        return 0;
    }
    
    int s3filer_read(const char *path, char *buf, size_t size, off_t offset,
                struct fuse_file_info *fi) 
    {
        // 1. Ваш ASCII-арт (содержимое файла)
        
        // 2. Определяем общую длину данных
        size_t len = file_mock_content.length();
        int res = 0;

        // 3. Проверяем, находится ли смещение (offset) за пределами данных
        if (offset < len) {
            // Определяем, сколько байтов остается для чтения после смещения
            size_t remaining = len - offset;
            
            // Определяем фактическое количество байтов для копирования.
            // Берем минимум из запрошенного размера (size) и оставшихся данных (remaining).
            if (remaining > size) {
                remaining = size;
            }

            // 4. Копируем данные из ASCII-арта (начиная со смещения offset) в буфер buf
            memcpy(buf, file_mock_content.c_str() + offset, remaining);
            
            // 5. Возвращаем количество скопированных байтов
            res = remaining;
        } 
        // Если offset >= len, возвращаем 0 (достигнут конец файла - EOF)
        
        return res;
    }    

    int s3filer_write (const char *path, const char *buf, size_t size, off_t offset,
		      struct fuse_file_info *)
    {
        file_mock_content = buf;
        return size;
    }

    int s3filer_release (const char *path, struct fuse_file_info *fi) {
        return 0;
    }

    int s3filer_create (const char *path, mode_t mode, struct fuse_file_info *fi) {
        // 2. Создаем файл с именем, указанным в path
        try {
            // Мы добавляем фактический путь в наш mock-список
            if (strlen(path) <= 1) {
                return -EIO;
            }
            files_mock.push_back(std::string(path + 1)); 
            
            // 3. ИНИЦИАЛИЗАЦИЯ СЕССИИ
            // Создаем объект сессии (или просто используем ID) для записи
            uint64_t file_handle_id = files_mock.size() - 1; 
            
            // 4. ВОЗВРАЩАЕМ ХЕНДЛ ЯДРУ
            fi->fh = file_handle_id; 

            // 5. Успех
            return 0;
        } catch (const std::exception& e) {
            // 6. Ошибка
            return -EIO; // Возвращаем ошибку Input/Output
        }
    }

    int s3filer_utimens (const char *, const struct timespec tv[2],
			 struct fuse_file_info *fi)
    {
        return 0;
    }
    static const struct fuse_operations s3filer_oper = {
        .getattr    = s3filer_getattr,
        .readlink   = NULL,
        .mknod      = NULL,
        .mkdir      = NULL,
        .unlink     = s3filer_unlink,
        .rmdir      = NULL,
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
        .readdir    = s3filer_readdir,
        .create     = s3filer_create,
        .utimens    = s3filer_utimens,
    };
}