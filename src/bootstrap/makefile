work_dir := $(shell pwd)
CXX := g++  # Default compiler
CXXFLAGS := -std=c++11 -Wall -Wextra
LDFLAGS :=
arch := x86_64-pc-linux-gnu

ifdef linux_arm64
	CXX := aarch64-linux-gnu-g++
	arch := aarch64-linux-gnu
endif


all: test

#test is working - .so files had to be deleted !!!!
curl_test:
	$(CXX) \
	-I$(work_dir)/depends/dist/include \
	-L$(work_dir)/depends/dist/lib \
	-L$(work_dir)/depends/dist/lib64 \
	-o curl_test curl_test.cpp \
	-lcurl -lssl -lcrypto -lz

#linux_x86=true make test
test:
	$(CXX) -DBOOTSTRAPTEST \
	-I$(work_dir)/../../depends/$(arch)/include \
	-L$(work_dir)/../../depends/$(arch)/lib \
	-L$(work_dir)/../../depends/$(arch)/lib64 \
	-o test test.cpp bootstrap.h bootstrap.cpp minizip/unzip.h minizip/unzip.c minizip/ioapi.h minizip/ioapi.c \
	-lcurl -pthread -lz -lssl -lcrypto -lboost_system-mt-x64 -lboost_filesystem-mt-x64 

docker_macos_test:
	/DSW/depends/x86_64-apple-darwin14/native/bin/clang++ -DBOOTSTRAPTEST \
	-target x86_64-apple-darwin14 -mmacosx-version-min=10.10 --sysroot /DSW/depends/SDKs/MacOSX10.11.sdk -mlinker-version=253.9 -stdlib=libc++ \
	-I/DSW/depends/x86_64-apple-darwin14/include \
	-L/DSW/depends/x86_64-apple-darwin14/lib \
	-L/DSW/depends/x86_64-apple-darwin14/lib64 \
	test.cpp logging.h tinyformat.h bootstrap.h bootstrap.cpp minizip/unzip.h minizip/unzip.c minizip/ioapi.h minizip/ioapi.c \
	-lcurl -DCURL_STATICLIB `/DSW/depends/x86_64-apple-darwin14/bin/curl-config --cflags --static-libs` -pthread -lz -lssl -lcrypto -lboost_system-mt-x64 -lboost_filesystem-mt-x64 

docker_linux_test:
	$(CXX) -DBOOTSTRAPTEST \
	-I/DSW/depends/x86_64-pc-linux-gnu/include \
	-L/DSW/depends/x86_64-pc-linux-gnu/lib \
	-L/DSW/depends/x86_64-pc-linux-gnu/lib64 \
	-o test test.cpp logging.h tinyformat.h bootstrap.h bootstrap.cpp minizip/unzip.h minizip/unzip.c minizip/ioapi.h minizip/ioapi.c \
	-lcurl -pthread -lz -lssl -lcrypto -ldl -lboost_system-mt-x64 -lboost_filesystem-mt-x64 

docker_windows_test:
	/usr/bin/x86_64-w64-mingw32-g++ -DBOOTSTRAPTEST -static \
	-I/DSW/depends/x86_64-w64-mingw32/include \
	-L/DSW/depends/x86_64-w64-mingw32/lib \
	-L/DSW/depends/x86_64-w64-mingw32/lib64 \
	-o test.exe test.cpp logging.h tinyformat.h bootstrap.h bootstrap.cpp minizip/unzip.h minizip/unzip.c minizip/ioapi.h minizip/ioapi.c \
	-lcurl -DCURL_STATICLIB `/DSW/depends/x86_64-w64-mingw32/bin/curl-config --cflags --static-libs` -pthread -lz -lssl -lcrypto -lboost_system-mt-s-x64 -lboost_filesystem-mt-s-x64 

clean:
	rm test test.exe curl_test bootstrap.zip