#!/bin/bash

# Sets the number of parallel jobs that can be used on the build process
# Recomended values: 1 job per 1GB of free RAM or 1 job per CPU physical core 
CPU_CORES=${CPU_CORES:-""}

# Architectures that can be built:
# linux-x64 
# linux-arm64
# macos-x64
# windows-x64
ARCHITECTURE=${ARCHITECTURE:-""}

# Get the origin URL
ORIGIN_URL=$(git config --get remote.origin.url)

# Extract the github username and repository name
if [[ $ORIGIN_URL =~ ^https://github.com/(.+)/(.+)\.git$ ]]; then
    GITHUB_USER="${BASH_REMATCH[1]}"
    GITHUB_REPO="${BASH_REMATCH[2]}"
elif [[ $ORIGIN_URL =~ ^git@github.com:(.+)/(.+)\.git$ ]]; then
    GITHUB_USER="${BASH_REMATCH[1]}"
    GITHUB_REPO="${BASH_REMATCH[2]}"
else
    echo "Unable to parse origin URL: $ORIGIN_URL"
    exit 1
fi

# Sets variables needed for the build
TICKER=${TICKER:-"${GITHUB_REPO}"}
UI_NAME=${UI_NAME:-"__Decenomy__"}
BASE_NAME=${BASE_NAME:-"__decenomy__"}

# Sets the build environment variable
#   0: The build will use the builder image available on docker hub
#   1: The build will use a locally build image for the builder image  
BUILD=${BUILD:-"0"}

# Sets the verify environment variable
#   0: The wallet is only built but not verified
#   1: The wallet is built at the same commit as the published release on GitHub
#	   and then verified against the published hash files
#   2: The wallet is built at the same commit as the published release on GitHub
#      and then all the files are downloaded and checked against the local ones   
VERIFY=${VERIFY:-"0"}

RED="\e[31m"
GREEN="\e[32m"
YELLOW="\e[33m"
BLUE="\e[34m"
RESET="\e[0m"

echo -e "${BLUE}
██████╗ ███████╗██╗    ██╗
██╔══██╗██╔════╝██║    ██║
██║  ██║███████╗██║ █╗ ██║
██║  ██║╚════██║██║███╗██║
██████╔╝███████║╚███╔███╔╝
╚═════╝ ╚══════╝ ╚══╝╚══╝ 
 DECENOMY Standard Wallet
Deterministic build script
${RESET}"

error() {
	echo -e "${RED}
███████╗██████╗ ██████╗  ██████╗ ██████╗ 
██╔════╝██╔══██╗██╔══██╗██╔═══██╗██╔══██╗
█████╗  ██████╔╝██████╔╝██║   ██║██████╔╝
██╔══╝  ██╔══██╗██╔══██╗██║   ██║██╔══██╗
███████╗██║  ██║██║  ██║╚██████╔╝██║  ██║
╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═╝
${YELLOW}$1
${RESET}"
	exit 1
}

info() {
	echo -e "${GREEN}$1${RESET}"
}

warn() {
	echo -e "${YELLOW}$1${RESET}"
}

trace() {
	echo -e "${BLUE}$1${RESET}"
}

# Array of commands to test
commands=(
	"docker" "wget" "curl" "jq" "lscpu" "grep" 
	"awk" "sha256sum" "unzip" "basename" "free"
)

# Loop through the array and check each command
for cmd in "${commands[@]}"
do
    if ! command -v $cmd &> /dev/null
    then
        error "${cmd} is not installed, please install before running again."
    fi
done

# Check if docker buildx is installed
docker buildx version &> /dev/null
if [ $? -ne 0 ]; then
	error "Docker Buildx is not installed, please install before running again."
fi

# Check if the CPU_CORES environment variable needs to be calculated
if [ "$CPU_CORES" = "" ]; then

	# Getting the number of sockets
	sockets=$(lscpu | grep "Socket(s):" | awk '{print $2}')

	# Getting the number of cores per socket
	cores_per_socket=$(lscpu | grep "Core(s) per socket:" | awk '{print $4}')

	# Calculating total physical cores
	total_physical_cores=$((sockets * cores_per_socket))

	# Getting the free RAM in gigabytes
	free_ram_gb=$(free --giga | awk '/Mem:/ {print $7}')

	trace "
Total sockets (physical CPUs): $sockets
Cores per socket: $cores_per_socket
Total physical cores: $total_physical_cores
Free ram (GB): $free_ram_gb"

	# Calculating the default number of CPUs to use
	CPU_CORES=$(
		if [ $total_physical_cores -le $free_ram_gb ]; then
			echo "$total_physical_cores"
		else
			echo "$free_ram_gb"
		fi
	)
fi

info "Using $CPU_CORES cores"

# Check if the ARCHITECTURE environment variable needs to be chosen
if [ "$ARCHITECTURE" = "" ]; then
	warn "Select an architecture from the list:"

	# List of architectures
	options=("linux-x64" "linux-arm64" "macos-x64" "windows-x64")

	# Use select to present a menu and store the selection in a variable
	select opt in "${options[@]}"
	do
		case $opt in
            "linux-x64"|"linux-arm64"|"macos-x64"|"windows-x64")
                break
                ;;
            *)
                error "Invalid option $REPLY"
                ;;
        esac
	done

	# Store the selection in a variable
	ARCHITECTURE=$opt
fi

info "Selected architecture: $ARCHITECTURE"

WALLET_DOCKER_FILE="./contrib/docker/Dockerfile.dsw-$ARCHITECTURE-wallet"

cp $WALLET_DOCKER_FILE $WALLET_DOCKER_FILE.tmp

# Check if the FULL_BUILD environment variable is already set
if [ "$BUILD" = "1" ]; then
	# The value of FULL_BUILD is 1, proceed with Docker build for the builder
	docker buildx build \
		--no-cache \
		--build-arg CPU_CORES=$CPU_CORES \
		-t decenomy/dsw-$ARCHITECTURE-builder \
		-f ./contrib/docker/Dockerfile.dsw-$ARCHITECTURE-builder .

	if [ $? -ne 0 ]; then
		error "Error: Docker build failed."
	fi

	sed -i "s|^FROM decenomy/dsw-$ARCHITECTURE-builder.*|FROM decenomy/dsw-$ARCHITECTURE-builder:latest|" "$WALLET_DOCKER_FILE.tmp"
fi

if [ "$VERIFY" = "0" ]; then
	TARGET=$(git branch --show-current)
fi

if [ "$VERIFY" -ge 1 ]; then
	# Fetch the latest release data using GitHub API
	response=$(curl -s "https://api.github.com/repos/$GITHUB_USER/$GITHUB_REPO/releases/latest")

	# Parse JSON response to get the browser_download_url of the asset
	PACKAGE_URL=$(echo "$response" | jq -r ".assets[] | select(.name | ascii_downcase | test(\"$ARCHITECTURE.zip\")) | .browser_download_url")

	# Check if download_url is empty or null
	if [ -z "$PACKAGE_URL" ] || [ "$PACKAGE_URL" == "null" ]; then
		error "No matching asset found."
	fi

	trace "Download URL for the package asset is: $PACKAGE_URL"

	# Parse JSON response to get the browser_download_url of the hash file asset
	PACKAGE_SHA256_URL=$(echo "$response" | jq -r ".assets[] | select(.name | ascii_downcase | test(\"$ARCHITECTURE.asc\")) | .browser_download_url")

	# Check if download_url is empty or null
	if [ -z "$PACKAGE_SHA256_URL" ] || [ "$PACKAGE_SHA256_URL" == "null" ]; then
		error "No matching asset found."
	fi

	trace "Download URL for the hash file asset is: $PACKAGE_SHA256_URL"

	sha256_file_contents=$(curl -sL "$PACKAGE_SHA256_URL")

	# Check if the download was successful
	if [ -z "$sha256_file_contents" ]; then
		error "Failed to download the file or the file is empty."
	fi

	TARGET=$(echo "$sha256_file_contents" | grep "commit" | awk '{print $1}')

	# Check if the target was successfully obtained
	if [ -z "$TARGET" ]; then
		error "Failed to get the target or the target is empty."
	fi
fi

image_tag=${TICKER,,}-${UI_NAME,,}-${TARGET,,}

docker buildx build \
	--no-cache \
	--build-arg CPU_CORES=$CPU_CORES \
	--build-arg TICKER=$TICKER \
	--build-arg NAME=$UI_NAME \
	--build-arg BASE_NAME=$BASE_NAME \
	--build-arg TARGET=$TARGET \
	--build-arg GITHUB_USER=$GITHUB_USER \
	-f $WALLET_DOCKER_FILE.tmp \
	-t $image_tag \
	 .

# Check if docker build was successful
if [ $? -ne 0 ]; then
    error "Error: Docker build failed."
fi

info "Successful build"

# Create a temporary container from the image
container_id=$(docker create $image_tag)

trace "Container ID: $container_id"

# Copy files from the container to the current directory
mkdir -p deploy
rm -rf deploy/$ARCHITECTURE
docker cp "$container_id":/${GITHUB_USER}/${TICKER}/deploy/. ./deploy/

# Main verification process
if [ "$VERIFY" -ge 1 ]; then
    files_to_check=("${ARCHITECTURE}.zip" "${BASE_NAME}d" "${BASE_NAME}-cli" "${BASE_NAME}-tx" "${BASE_NAME}-qt")

    for file_to_check in "${files_to_check[@]}"; do
		remote_hash="$(echo "$sha256_file_contents" | grep -i "${file_to_check}" | awk '{print $1}')"
		local_hash="$(sha256sum "$(find "./deploy/$ARCHITECTURE" -type f | grep -i "${file_to_check}")" | awk '{print $1}')"

		if [ "$remote_hash" != "$local_hash" ]; then
			error "${file_to_check^} file hashes don't match"
		fi
    done

    if [ "$VERIFY" -ge 2 ]; then
        rm -rf "deploy/$ARCHITECTURE/download"
        mkdir -p "deploy/$ARCHITECTURE/download"
        cd "deploy/$ARCHITECTURE/download"

        wget "$PACKAGE_URL"
        filename=$(basename "$PACKAGE_URL")
        unzip "$filename"

        for file_to_check in "${files_to_check[@]}"; do
			filename="$(find "./" -type f | grep -i "$file_to_check" | xargs -n1 basename)"
            download_hash="$(sha256sum "./$filename" | awk '{print $1}')"
            local_hash="$(sha256sum "../$filename" | awk '{print $1}')"

            if [ "$download_hash" != "$local_hash" ]; then
				error "Download $file_to_check file hashes don't match"
            fi
        done

		cd ../../..
    fi

    info "Binaries verified successfully"
fi

info "
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⣦⣤⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀   
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣼⣿⣿⣿⡆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀   
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣿⣿⣿⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀   
⠀⠀⠀⠀⠀⠀⠀⠀⣠⣾⣿⣿⣿⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀   
⢠⣤⣤⣤⡀⠀⢀⣼⣿⣿⣿⣿⣿⣿⣧⣤⣤⣤⣤⣤⣤⣤⣤⣄⡀⠀
⢸⣿⣿⣿⡇⠀⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡄
⢸⣿⣿⣿⡇⠀⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇
⢸⣿⣿⣿⡇⠀⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠀
⢸⣿⣿⣿⡇⠀⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠁⠀
⢸⣿⣿⣿⡇⠀⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠁⠀⠀
⢸⣿⣿⣿⡇⠀⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠃⠀⠀⠀
⢸⣿⣿⣿⡇⠀⠈⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠃⠀⠀⠀⠀
"