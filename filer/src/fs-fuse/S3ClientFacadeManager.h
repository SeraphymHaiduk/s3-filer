#include <memory>
#include <S3ClientFacade.h>

namespace S3Filer {
    // TODO: move to S3Client module
    class S3ClientFacadeManager
    {
    private:
        static std::shared_ptr<S3ClientFacadeBase> s_s3Client;
    public:
        static void setS3ClientFacade(std::shared_ptr<S3ClientFacadeBase> client)
        {
            s_s3Client = client;
        }
        static S3ClientFacadeBase *getS3ClientFacade()
        {
            if (!s_s3Client) {
                s_s3Client = std::make_shared<S3ClientFacade>();
            }
            return s_s3Client.get();
        }
    };
} // namespace S3Filer
