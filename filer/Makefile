
.PHONY: clean build init-submodules


dep:
	sudo apt update
	sudo apt install -y \
		libcurl4-openssl-dev \
		libfuse3-dev

build:
	cmake -B build \
	  -DCMAKE_C_COMPILER=/usr/bin/gcc \
	  -DCMAKE_CXX_COMPILER=/usr/bin/g++
	cmake --build build
	
clean:
	rm -rf build

init-submodules:
	git submodule update --init --recursive
