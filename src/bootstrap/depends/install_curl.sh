#!/bin/bash
work_dir=$(pwd)

# Define default variables
install_dir="${work_dir}/dist"
openssl_dir="${work_dir}/openssl"

# Parse command line options
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        --prefix)
            install_dir="$2"
            shift # past argument
            shift # past value
            ;;
        --openssl)
            openssl_dir="$2"
            shift # past argument
            shift # past value
            ;;
        *)  # unknown option
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Define variables
curl_version="8.4.0"
curl_download_url="https://curl.se/download/curl-${curl_version}.tar.gz"
curl_tar="curl-${curl_version}.tar.gz"
curl_dir="curl-${curl_version}"

# Download curl source
# Check if the file exists
if [ ! -f "$curl_tar" ]; then
	wget "$curl_download_url"
	if [ $? -ne 0 ]; then
	    echo "Failed to download curl source"
	    exit 1
	fi
fi

if [ ! -d "$curl_dir" ]; then
	# Extract the downloaded tarball
	tar -xf "$curl_tar"
	if [ $? -ne 0 ]; then
	    echo "Failed to extract curl source"
	    exit 1
	fi
fi

# Configure curl with OpenSSL support if OpenSSL path is provided
cd "$curl_dir" || exit 1
if [ -n "$openssl_dir" ]; then
	echo "Using ssl: $openssl_dir"
    ./configure --prefix="$install_dir" --with-ssl="$openssl_dir"
else
    ./configure --prefix="$install_dir"
fi

if [ $? -ne 0 ]; then
    echo "Failed to configure curl"
    exit 1
fi

# Compile and install curl
make -j2
if [ $? -ne 0 ]; then
    echo "Failed to compile curl"
    exit 1
fi

make install
if [ $? -ne 0 ]; then
    echo "Failed to install curl"
    exit 1
fi

echo "curl successfully installed to $install_dir"
