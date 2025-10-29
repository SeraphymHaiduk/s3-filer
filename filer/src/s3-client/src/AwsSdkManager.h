#include <aws/core/Aws.h>

namespace S3Filer {
    class AwsSdkManager {
    private:
        Aws::SDKOptions m_options;
        AwsSdkManager();
        ~AwsSdkManager();
    public:
        static void init();
    };
}