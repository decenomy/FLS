work_dir := $(shell pwd)
CXX := g++  # Default compiler
CXXFLAGS := -std=c++11 -Wall -Wextra
LDFLAGS :=
arch := x86_64-pc-linux-gnu

dist_dir := $(work_dir)/depends/dist

# Rule to create or clean the dist directory
prepare_dist:
	rm -rf $(dist_dir); \
	mkdir -p $(dist_dir); \
	echo "Cleaned and created directory $(dist_dir)"; \

ifdef macos
	CXX := /DSW/depends/x86_64-apple-darwin14/native/bin/clang++
	arch := x86_64-apple-darwin14
	#CXXFLAGS += -target x86_64-apple-darwin14 -mmacosx-version-min=10.10 --sysroot /DSW/depends/SDKs/MacOSX10.11.sdk -mlinker-version=253.9 -stdlib=libc++ -I/DSW/depends/x86_64-apple-darwin14/include
	#LDFLAGS += -L/DSW/depends/x86_64-apple-darwin14/lib -lboost_system-mt-x64 -lboost_filesystem-mt-x64 -lz -lcurl -DCURL_STATICLIB `/DSW/depends/x86_64-apple-darwin14/bin/curl-config --cflags --static-libs`
endif

ifdef windows
	CXX := /usr/bin/x86_64-w64-mingw32-g++
	arch := x86_64-w64-mingw32
	#CXXFLAGS += -I/DSW/depends/x86_64-w64-mingw32/include/curl -I/DSW/depends/x86_64-w64-mingw32/include
	#LDFLAGS += -L/DSW/depends/x86_64-w64-mingw32/lib -lboost_system-mt-s-x64 -lboost_filesystem-mt-s-x64 -lz -lcurl -DCURL_STATICLIB `/DSW/depends/x86_64-w64-mingw32/bin/curl-config --cflags --static-libs`

endif

ifdef linux_arm64
	CXX := aarch64-linux-gnu-g++
	arch := aarch64-linux-gnu
endif

ifdef linux_x86
	CXX := g++
	arch := x86_64-pc-linux-gnu
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
	$(CXX) \
	-I$(work_dir)/../../depends/$(arch)/include \
	-L$(work_dir)/../../depends/$(arch)/lib \
	-L$(work_dir)/../../depends/$(arch)/lib64 \
	-o test test.cpp bootstrap.h bootstrap.cpp minizip/unzip.h minizip/unzip.c minizip/ioapi.h minizip/ioapi.c \
	-lcurl -pthread -lz -lssl -lcrypto -lboost_system-mt-x64 -lboost_filesystem-mt-x64 

clean:
	rm test curl_test bootstrap.zip