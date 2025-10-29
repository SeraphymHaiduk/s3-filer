

## Run minIO on your S3 storage server
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


## Run filer on your filer-server
TODO

## Run SMB server on your filer-server
TODO

## Connect via SMB from the  client
TODO