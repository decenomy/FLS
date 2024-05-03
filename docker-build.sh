#!/bin/bash

# Check if all required arguments are provided
if [ $# -ne 6 ]; then
    echo "Usage: $0 <TICKER> <NAME> <BASE_NAME> <TARGET> <VERSION> <ARCHITECTURE>"
    echo "Usage: $0 DSW __Decenomy__ __decenomy__ develop 3.0.0.0 Linux"
    exit 1
fi

#Available architectures before 1.5.2.2 version
 # - Windows
 # - MacOS
 # - Linux
 # - aarch64
 # - aarch32

 #Available architectures after 
 # - linux-arm64
 # - linux-x64
 # - macos-x64
 # - windows-x64

# Assign command-line arguments to variables
TICKER="$1"
NAME="$2"
BASE_NAME="$3"
TARGET="$4"
VERSION="$5"
ARCHITECTURE="$6"
ARCHITECTURE2="linux-x64"

# Run docker build command and capture the output
docker build \
	--no-cache \
	--build-arg CPU_CORES=4 \
	--build-arg TICKER=$TICKER \
	--build-arg NAME=$NAME \
	--build-arg BASE_NAME=$BASE_NAME \
	--build-arg TARGET=$TARGET \
	-f ./contrib/docker/Dockerfile.dsw-${ARCHITECTURE2}-wallet \
	-t own_build . > build_output.txt

# Check if docker build was successful
if [ $? -ne 0 ]; then
    echo "Error: Docker build failed."
    exit 1
fi

# Create a temporary container from the image
container_id=$(docker create own_build)

echo "Container ID: $container_id"

# Copy files from the container to the current directory
docker cp "$container_id":/${TICKER}/deploy/. .

# Remove the temporary container
docker rm "$container_id"

if [ "$TICKER" == "DSW" ]; then
	echo "We are in DSW dev repo.."
	# Change directory to the second architecture
	cd "$ARCHITECTURE2" || { echo "Error: Unable to change directory to $ARCHITECTURE2"; exit 1; }
	
	FILENAME2="${TICKER}-${VERSION}-${ARCHITECTURE}-x64.zip"
	local_=$(sha256sum "$FILENAME2")
	cd ..
	echo "sha256 local: $local_"
else
	# Make HTTP request to GitHub API to get latest release info
	response=$(curl -s https://api.github.com/repos/decenomy/${TICKER}/releases/latest)

	# Parse JSON and extract browser_download_url
	browser_download_url=$(echo "$response" | jq -r --arg TICKER "$TICKER" --arg VERSION "$VERSION" --arg ARCHITECTURE "$ARCHITECTURE" '.assets[] | select(.name == "\($TICKER)-\($VERSION)-\($ARCHITECTURE).zip") | .browser_download_url')

	# Output the extracted URL
	echo "Browser Download URL: $browser_download_url"

	# Define the filename based on the provided arguments
	FILENAME="${TICKER}-${VERSION}-${ARCHITECTURE}.zip"
	FILENAME2="${TICKER}-${VERSION}-${ARCHITECTURE}-x64.zip"

	# Check if the file already exists
	if [ ! -e "$FILENAME" ]; then
		wget "$browser_download_url"
	fi
	# Download the file

	origin=$(sha256sum "$FILENAME")

	# Change directory to the second architecture
	cd "$ARCHITECTURE2" || { echo "Error: Unable to change directory to $ARCHITECTURE2"; exit 1; }
	local_=$(sha256sum "$FILENAME2")
	cd ..

	echo "sha256 origin: $origin"
	echo "sha256 local: $local_"

	if [ "$origin" = "$local_" ]; then
	    echo "Build Successfully"
	else
	    echo "Builds don't match"
	fi
fi
