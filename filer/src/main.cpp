
#include "AwsSdkManager.h"
#include "S3ClientFacade.h"
#include <iostream>
#include <FsFuse.h>
#include <S3ClientFacadeManager.h>

int main(int argc, char *argv[]) {
    S3Filer::AwsSdkManager::init();

    if (!S3Filer::S3ClientFacadeManager::getS3ClientFacade()->isConnectionEstablished()) {
        exit(1);
    }
        
    for (int i = 0; i < argc; i++) {
        std::cout << argv[i] << std::endl;
    }
    int ret = S3Filer::initS3Fuse(argc, argv);
    return ret;
}
