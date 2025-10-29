import pytest
import os

@pytest.fixture(scope="session", autouse=True)
def filesystem_mounting_exec():
    path = os.getenv("PROJECT_BIN_PATH")
    if path is None:
        raise Exception(f"path is None")
    yield path