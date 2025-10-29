
.PHONY: clean build init-submodules

# 1
init-submodules:
	git submodule update --init --recursive

# 2
dep:
	apt update
	apt install -y \
		libcurl4-openssl-dev \
		libfuse3-dev \
		python3.11 python3.11-venv

# 3.A - use for a standard dev build configuration
configure:
	cmake -B build \
	  -DCMAKE_C_COMPILER=/usr/bin/gcc \
	  -DCMAKE_CXX_COMPILER=/usr/bin/g++ \
	  -DBUILD_TESTS=ON

# 3.B - use for a bit faster build without tests
configure-no-tests:
	cmake -B build \
	  -DCMAKE_C_COMPILER=/usr/bin/gcc \
	  -DCMAKE_CXX_COMPILER=/usr/bin/g++ \
	  -DBUILD_TESTS=OFF

# 4 - Necessary for integration tests
create-env:
	python3.11 -m venv env
	./env/bin/python3.11 -m pip install -r requirements.txt

# 5
build:
	cmake --build build

# 6
clean:
	rm -rf build

test-integration:
	cd build/tests/integration && ctest --output-on-failure

test-unit:
	cd build/tests/unit && ctest --output-on-failure

test-integration-regular-fs:
# 	Target runs integration tests on regular filesystem 
#	ensures that tests by itself are correct since all should work on a regular filesystem
#	if something doesn't work on a regular filesystem you should fix tests
	USE_REGULAR_FS=ON cd build/tests/integration && ctest --output-on-failure

test: test-integration test-unit
	
