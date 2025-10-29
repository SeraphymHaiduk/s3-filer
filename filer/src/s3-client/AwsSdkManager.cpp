#include "AwsSdkManager.h"

using namespace S3Filer;

AwsSdkManager::AwsSdkManager()
{
    Aws::InitAPI(m_options);
}

AwsSdkManager::~AwsSdkManager()
{
    Aws::ShutdownAPI(m_options); 
}

void AwsSdkManager::init()
{
    static AwsSdkManager manager{};
}