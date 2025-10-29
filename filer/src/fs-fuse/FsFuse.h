#pragma once
#define FUSE_USE_VERSION 30

#include <fuse3/fuse_opt.h>
#include <fuse3/fuse.h>
#include <cstring>
#include <stdio.h>

#include <string>
#include <vector>

#include <memory>
#include <algorithm>
#include <sys/stat.h>

namespace S3Filer {
    class S3ClientFacadeBase;
    int initS3Fuse(int argc, char *argv[]);
}