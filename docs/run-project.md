

## Run minIO S3 storage 
```
docker pull minio/minio
```
```
docker run -p 9000:9000 -p 9001:9001 \
  -e MINIO_ROOT_USER=admin \
  -e MINIO_ROOT_PASSWORD=adminadmin \
  -v /data/minio:/data \
  minio/minio server /data --console-address ":9001"
```
9000 - S3 API port
9001 - web-console port

/data/minio - local path for data

NOTE: go to http://localhost:9001 and create a bucket 'development'

## Build
```
cd filer

make init-submodules
make dep

make configure
make build

```
## Run filesystem

For mounting
```
cd s3-filer/build
mkdir s3mount
S3Filer s3mount
```

For unmounting
```
fusermount -u s3mount

rmdir s3mount #optionally
```
## Testing
After building the project, running s3 storage and creating a bucket
prepare environment for integration tests, since they are written in python

```
cd s3-filer
make create-env
```

Then run all tests
```
make test
```

Only integration tests
```
make test-integration
```

Or only unit tests
```
make test-unit
```

---------------------

## Run filer on your filer-server
TODO

## Run SMB server on your filer-server
TODO

## Connect via SMB from the  client
TODO