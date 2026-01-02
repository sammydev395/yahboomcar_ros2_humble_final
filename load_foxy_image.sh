#!/bin/bash
# Script to load the downloaded ROS2 Foxy container image
# Handles both .tar and .tar.gz formats, and extracts from zip if needed

set -e

DOWNLOADS_DIR="$HOME/Downloads"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "=== ROS2 Foxy Container Image Loader ==="
echo ""

# Function to find the downloaded file
find_downloaded_file() {
    # Look for common patterns
    local patterns=(
        "*yahboom*"
        "*rosmaster*"
        "*x3plus*"
        "*foxy*"
        "*ros2*"
        "*docker*"
        "*.tar"
        "*.tar.gz"
        "*.zip"
    )
    
    for pattern in "${patterns[@]}"; do
        local file=$(find "$DOWNLOADS_DIR" -maxdepth 1 -type f -iname "$pattern" 2>/dev/null | head -1)
        if [ -n "$file" ] && [ ! -f "$file.crdownload" ]; then
            echo "$file"
            return 0
        fi
    done
    
    return 1
}

# Check if download is still in progress
check_download_status() {
    local crdownload_files=$(find "$DOWNLOADS_DIR" -maxdepth 1 -name "*.crdownload" 2>/dev/null)
    if [ -n "$crdownload_files" ]; then
        echo "Warning: Download appears to be in progress (.crdownload files found)"
        echo "Files:"
        echo "$crdownload_files"
        echo ""
        read -p "Wait for download to complete? (y/n) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            echo "Waiting for download to complete..."
            while find "$DOWNLOADS_DIR" -maxdepth 1 -name "*.crdownload" | grep -q .; do
                sleep 30
                echo "Still downloading... ($(date))"
            done
            echo "Download appears to be complete!"
        else
            echo "Exiting. Please run this script again after download completes."
            exit 1
        fi
    fi
}

# Handle zip file extraction
extract_zip_if_needed() {
    local file="$1"
    
    if [[ "$file" == *.zip ]]; then
        echo "Detected ZIP file. Extracting..."
        echo "This may take a while for large files..."
        
        local extract_dir="$SCRIPT_DIR/foxy_image_extract"
        mkdir -p "$extract_dir"
        
        # Check available disk space
        local available_space=$(df "$extract_dir" | tail -1 | awk '{print $4}')
        local file_size=$(stat -f%z "$file" 2>/dev/null || stat -c%s "$file" 2>/dev/null)
        
        echo "File size: $(numfmt --to=iec-i --suffix=B $file_size 2>/dev/null || echo "$file_size bytes")"
        echo "Available space: $(numfmt --to=iec-i --suffix=B $available_space 2>/dev/null || echo "$available_space bytes")"
        
        read -p "Proceed with extraction? (y/n) " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            echo "Extraction cancelled."
            exit 1
        fi
        
        echo "Extracting to: $extract_dir"
        unzip -q "$file" -d "$extract_dir" || {
            echo "Error: Extraction failed. Trying with verbose output..."
            unzip "$file" -d "$extract_dir"
        }
        
        # Find the actual image file in extracted contents
        local image_file=$(find "$extract_dir" -type f \( -name "*.tar" -o -name "*.tar.gz" -o -name "*.img" \) | head -1)
        
        if [ -n "$image_file" ]; then
            echo "Found image file in extracted contents: $image_file"
            echo "$image_file"
            return 0
        else
            echo "Warning: No .tar or .tar.gz file found in extracted contents"
            echo "Contents:"
            ls -lh "$extract_dir" | head -20
            return 1
        fi
    else
        echo "$file"
        return 0
    fi
}

# Main execution
check_download_status

# Find the downloaded file
echo "Searching for downloaded image file in: $DOWNLOADS_DIR"
IMAGE_FILE=$(find_downloaded_file)

if [ -z "$IMAGE_FILE" ]; then
    echo "Error: Could not find downloaded image file."
    echo ""
    echo "Please specify the file path manually:"
    read -p "Enter full path to image file: " IMAGE_FILE
    
    if [ ! -f "$IMAGE_FILE" ]; then
        echo "Error: File not found: $IMAGE_FILE"
        exit 1
    fi
else
    echo "Found: $IMAGE_FILE"
fi

# Check file size
FILE_SIZE=$(stat -f%z "$IMAGE_FILE" 2>/dev/null || stat -c%s "$IMAGE_FILE" 2>/dev/null)
echo "File size: $(numfmt --to=iec-i --suffix=B $FILE_SIZE 2>/dev/null || echo "$FILE_SIZE bytes")"
echo ""

# Handle zip extraction if needed
if [[ "$IMAGE_FILE" == *.zip ]]; then
    IMAGE_FILE=$(extract_zip_if_needed "$IMAGE_FILE")
    if [ -z "$IMAGE_FILE" ] || [ ! -f "$IMAGE_FILE" ]; then
        echo "Error: Could not extract or find image file"
        exit 1
    fi
fi

# Determine file type and load accordingly
echo "Loading Docker image..."
echo ""

if [[ "$IMAGE_FILE" == *.tar.gz ]] || [[ "$IMAGE_FILE" == *.tgz ]]; then
    echo "Loading compressed tar archive..."
    docker load -i "$IMAGE_FILE"
elif [[ "$IMAGE_FILE" == *.tar ]]; then
    echo "Loading tar archive..."
    docker load -i "$IMAGE_FILE"
else
    echo "Warning: Unknown file type. Attempting to load as Docker image..."
    docker load -i "$IMAGE_FILE" || {
        echo "Error: Failed to load image. File may not be a valid Docker image."
        echo "Supported formats: .tar, .tar.gz"
        exit 1
    }
fi

if [ $? -eq 0 ]; then
    echo ""
    echo "=== Image Loaded Successfully ==="
    echo ""
    echo "Available Docker images:"
    docker images | head -10
    echo ""
    echo "Next steps:"
    echo "1. Identify the loaded image name/tag"
    echo "2. Run extraction script:"
    echo "   ./extract_foxy_container_info.sh <image_name_or_container_name>"
    echo ""
    echo "To find the image name, look for the 'Loaded image:' message above,"
    echo "or run: docker images"
else
    echo ""
    echo "=== Failed to Load Image ==="
    exit 1
fi

