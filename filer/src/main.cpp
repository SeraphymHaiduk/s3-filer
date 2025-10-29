
#include "AwsSdkManager.h"
#include "FilerS3ClientWrapper.h"
#include <iostream>
#include <FsFuse.h>

using namespace S3Filer;

    // FilerS3Client::instance().uploadFile("animals", "file.txt");

int main(int argc, char *argv[]) {
    AwsSdkManager::init();
    if (!FilerS3ClientWrapper::instance().isConnectionEstablished())
        exit(1);

    for (int i = 0; i < argc; i++) {
        std::cout << argv[i] << std::endl;
    }
    int ret = initS3Fuse(argc, argv);
    return ret;
}
