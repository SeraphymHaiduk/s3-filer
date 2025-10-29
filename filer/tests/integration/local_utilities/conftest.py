import pytest
import os
import shutil
import subprocess
from pathlib import Path
import boto3

test_dir: Path = Path("test-dir")
mount_dir: Path = Path(test_dir, "s3mount")

USE_REGULAR_FS: str | None = os.getenv("USE_REGULAR_FS")
useRegularFS: bool = USE_REGULAR_FS is not None and USE_REGULAR_FS == "ON"


@pytest.fixture(scope="session", autouse=True)
def mounting_point(filesystem_mounting_exec: str):

    if not os.path.exists(test_dir):
        os.mkdir(test_dir)
    
    if not os.path.exists(mount_dir):
        os.mkdir(mount_dir)
    else:
        # Try to unmount if filesystem is mounted for a some reason
        subprocess.run(["fusermount", "-uz", mount_dir], check=False)

    if useRegularFS != True:
        print("Using S3 FUSE filesystem")
        subprocess.run([filesystem_mounting_exec, mount_dir], check=False)
    else:
        print("Using REGULAR filesystem")

    yield mount_dir
    
    if os.path.exists(mount_dir):
        if useRegularFS != True:
            subprocess.run(["fusermount", "-uz", mount_dir])
    
    if os.path.exists(test_dir):
        shutil.rmtree(test_dir)


@pytest.fixture(scope="function", autouse=True)
def clear_mounting_poit():

    # NOTE: Cleares bucket before testings
    def clear_bucket():
        BUCKET_NAME = "development"
        s3_resource = boto3.resource(
            's3',
            endpoint_url="http://localhost:9000",
            aws_access_key_id="admin",
            aws_secret_access_key="adminadmin",
        )

        s3_resource.Bucket(BUCKET_NAME).objects.all().delete()

    # NOTE: Clears dir before testing. (FOR REGULAR FILESYSTEM ONLY)
    def clear_dir():
        shutil.rmtree(test_dir)


    clear_bucket()


class CommandResponse:
    def __init__(self, code: int, stdout: str, stderr: str):
        self.code = code
        self.stdout = stdout
        self.stderr = stderr

def run_command(command: str | list[str]) -> CommandResponse:
    """Runs command and return error code with logs (exit_code, stdout, stderr)"""
    result = subprocess.run(
        command,
        shell=True,
        capture_output=True,
        text=True,
        check=False # NOTE: Don't raise python exception in case or command error
    )
    return CommandResponse(result.returncode, result.stdout, result.stderr)