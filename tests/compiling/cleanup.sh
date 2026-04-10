#!/bin/bash
# This script file was generated using Google Gemini

set -e

# --- Color Definitions ---
BLUE='\033[0;34m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${BLUE}🧹 Starting test environment cleanup...${NC}"

# 1. Delete the Docker image (-f forces it even if a stopped container exists)
echo -e "🐳 Removing Docker image 'vkg_gen_test_env'..."
if docker image inspect vkg_gen_test_env > /dev/null 2>&1; then
    docker rmi -f vkg_gen_test_env
    echo -e "${GREEN}   Image removed.${NC}"
else
    echo -e "${YELLOW}   Image not found. Already clean.${NC}"
fi

# 2. Navigate to where this script is located
TEST_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$TEST_DIR"

# 3. Nuke the build directories
echo -e "🗑️  Removing build directories..."
rm -rf build_*

# 4. Nuke the logs
echo -e "📜 Removing logs directory..."
rm -rf logs/

# 5. Clean up stray Dockerfiles (just in case the setup script crashed halfway)
rm -f Dockerfile.matrix

echo -e "${GREEN}✨ Cleanup complete! Your workspace is spotless.${NC}"
