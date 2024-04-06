#!/bin/bash
work_dir=$(pwd)

# Define variables
openssl_version="3.2.1"
openssl_download_url="https://github.com/openssl/openssl/releases/download/openssl-3.2.1/openssl-${openssl_version}.tar.gz"
openssl_tar="openssl-${openssl_version}.tar.gz"
openssl_dir="openssl-${openssl_version}"
install_dir="${work_dir}/dist/"

# Check if the file exists
if [ ! -f "$openssl_tar" ]; then
    # Download OpenSSL source
    wget "$openssl_download_url"
    if [ $? -ne 0 ]; then
        echo "Failed to download OpenSSL source"
    fi
    exit 1
fi

if [ ! -d "$openssl_dir" ]; then
    # Extract the downloaded tarball
    tar -xf "$openssl_tar"
    if [ $? -ne 0 ]; then
        echo "Failed to extract OpenSSL source"
        exit 1
    fi
fi

echo "Compiling openssl: ${openssl_version}"
# Configure OpenSSL
cd "$openssl_dir" || exit 1
./config --prefix="$install_dir" --openssldir="$install_dir"
if [ $? -ne 0 ]; then
    echo "Failed to configure OpenSSL"
    exit 1
fi

# Compile and install OpenSSL
make -j2
if [ $? -ne 0 ]; then
    echo "Failed to compile OpenSSL"
    exit 1
fi

make install_sw
if [ $? -ne 0 ]; then
    echo "Failed to install OpenSSL"
    exit 1
fi

echo "OpenSSL successfully installed to $install_dir"
