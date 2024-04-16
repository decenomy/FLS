
TAG=windows-x64-builder

#To compile and build the container execute docker in this way:
docker build \
	-f Dockerfile.windows-x64 \
	-t $TAG .

# Check if docker build was successful
if [ $? -ne 0 ]; then
    echo "Error: Docker build failed."
    exit 1
fi

# Create a temporary container from the image
container_id=$(docker create $TAG)

echo "Container ID: $container_id"

# Copy files from the container to the current directory
docker cp "$container_id":home/app/test.exe .

# Remove the temporary container
docker rm "$container_id"


# To run the container use:
#docker run \
#	-it \
#	-v "$(pwd):/home/app" \
#	${TAG}:latest
