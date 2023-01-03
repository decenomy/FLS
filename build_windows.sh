#!/bin/bash
echo -e "\033[0;32mHow many CPU cores do you want to be used in compiling process? (Default is 1. Press enter for default.)\033[0m"
read -e CPU_CORES
if [ -z "$CPU_CORES" ]
then
	CPU_CORES=1
fi

# Upgrade the system and install required dependencies
	sudo apt update
	sudo apt install git zip unzip build-essential libtool bsdmainutils autotools-dev autoconf pkg-config automake python3 curl g++-mingw-w64-x86-64 g++-mingw-w64-x86-64-posix libqt5svg5-dev -y
	echo "1" | sudo update-alternatives --config x86_64-w64-mingw32-g++

# Clone code from official Github repository
	rm -rf FLS
	git clone https://github.com/decenomy/FLS.git

# Entering directory
	cd FLS

# Disable WSL support for Win32 applications.
sudo bash -c "echo 0 > /proc/sys/fs/binfmt_misc/status"

# Compile dependencies
	cd depends 
	make -j$(echo $CPU_CORES) HOST=x86_64-w64-mingw32 
	cd ..

# Compile
	./autogen.sh
	./configure --prefix=$(pwd)/depends/x86_64-w64-mingw32 --disable-debug --disable-tests --disable-bench --disable-online-rust --enable-upnp-default CFLAGS="-O3" CXXFLAGS="-O3"
	make -j$(echo $CPU_CORES) HOST=x86_64-w64-mingw32
	cd ..

# Create zip file of binaries
	cp FLS/src/flitsd.exe FLS/src/flits-cli.exe FLS/src/flits-tx.exe FLS/src/qt/flits-qt.exe .
	zip FLS-Windows.zip flitsd.exe flits-cli.exe flits-tx.exe flits-qt.exe
	rm -f flitsd.exe flits-cli.exe flits-tx.exe flits-qt.exe

# Enable WSL support for Win32 applications.
	sudo bash -c "echo 1 > /proc/sys/fs/binfmt_misc/status"
