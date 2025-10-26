// #include "FilerS3Client.h"
// #include "AwsSdkManager.h"
#include <iostream>
#include <FsFuse.h>

using namespace S3Filer;

// AwsSdkManager::init();
    // FilerS3Client::instance().uploadFile("animals", "file.txt");

int main(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        std::cout << argv[i] << std::endl;
    }
    int ret = fuse_main(argc, argv, &s3filer_oper, NULL);
    return ret;
}
