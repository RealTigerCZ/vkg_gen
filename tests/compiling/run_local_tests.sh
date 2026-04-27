#!/bin/bash
# This script file was generated using Google Gemini

set -e

TEST_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$TEST_DIR/../.." && pwd)"

# --- Color Definitions ---
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}===================================================${NC}"
echo -e "${CYAN}🚀 Starting Local Configuration Tests from: $TEST_DIR${NC}"
echo -e "${CYAN}===================================================${NC}"

# Check for required tools
for tool in cmake ninja c++; do
    if ! command -v $tool &> /dev/null; then
        echo -e "${RED}❌ Required tool '$tool' is not installed locally. Aborting.${NC}"
        exit 1
    fi
done

# --- 🧠 SMART RESOURCE CALCULATION (Local Machine) ---
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS fallback
    CORES=$(sysctl -n hw.physicalcpu)
    RAM_GB=$(($(sysctl -n hw.memsize) / 1024 / 1024 / 1024))
else
    # Linux
    CORES=$(lscpu -b -p=Core,Socket 2>/dev/null | grep -v "^#" | sort -u | wc -l || nproc)
    RAM_GB=$(awk '/MemTotal/ {printf "%.0f", $2/1000/1000}' /proc/meminfo) # use 1000 instead of 1024, because of reserved memory.
fi

RAM_LIMIT=$(( RAM_GB / 4 ))
SYSTEM_CAPACITY=$CORES
if [ "$RAM_LIMIT" -lt "$SYSTEM_CAPACITY" ]; then SYSTEM_CAPACITY=$RAM_LIMIT; fi
if [ "$SYSTEM_CAPACITY" -lt 1 ]; then SYSTEM_CAPACITY=1; fi

# Since we run these sequentially (not backgrounded), Ninja gets the full capacity
NINJA_JOBS=$SYSTEM_CAPACITY

echo -e "${CYAN}📊 Local Stats: ${CORES} Physical Cores, ${RAM_GB}GB RAM.${NC}"
echo -e "${CYAN}⚙️  Running sequential builds with ${YELLOW}${NINJA_JOBS} job(s)${CYAN} per build.${NC}"
echo -e "${CYAN}---------------------------------------------------${NC}"

# Ensure we are operating inside the tests/compiling directory
cd "$TEST_DIR"

# Setup logs directory
mkdir -p logs
rm -f logs/local_*.log

CONFIGS=("Debug" "Release" "RelWithDebInfo")
FAIL=0

for CONFIG in "${CONFIGS[@]}"; do

    CONFIG_LOWER="${CONFIG,,}"
    BUILD_DIR="build_local_${CONFIG_LOWER}"
    LOG_FILE="logs/local_${CONFIG_LOWER}.log"

    echo -e "${CYAN}🧪 Testing Configuration: ${YELLOW}$CONFIG${NC}"

    # Run everything in a subshell to capture the output into the log file cleanly
    if (
        set -e # Ensure the subshell fails if any command inside fails

        rm -rf "$BUILD_DIR"

        # 1. Configure (Point -S to ROOT_DIR since we are in TEST_DIR)
        cmake -G Ninja -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=$CONFIG \
              -DCMAKE_COLOR_DIAGNOSTICS=ON

        # 2. Build
        ninja -C "$BUILD_DIR" -j $NINJA_JOBS

        # 3. Setup Execution
        cp "$ROOT_DIR/ref/vk.xml" "$BUILD_DIR/"
        cd "$BUILD_DIR"

        # 4. Execute Generator
        ./vkgen

        # 5. Verify against Golden Outputs (relative to BUILD_DIR)
        diff -q vkg.hpp ../golden/vkg.hpp
        diff -q vkg.cpp ../golden/vkg.cpp
        diff -q vkg.cppm ../golden/vkg.cppm

        # 6. Execute Test Program
        cp ../golden/vkg_comp.cpp .
        ninja vkg_comp -j $NINJA_JOBS
        ./vkg_comp

        cp ../golden/modules.cpp .
        ninja modules -j $NINJA_JOBS
        ./modules_test

    ) > "$LOG_FILE" 2>&1; then
        echo -e "${GREEN}✅ $CONFIG Passed!${NC}"
    else
        echo -e "${RED}❌ $CONFIG Failed! Check $LOG_FILE for details.${NC}"
        FAIL=1
    fi

    echo -e "${CYAN}---------------------------------------------------${NC}"
done

if [ $FAIL -ne 0 ]; then
    echo -e "${RED}🔥 Some local configurations failed!${NC}"
    exit 1
else
    echo -e "${GREEN}🎉 All local configurations passed perfectly!${NC}"
fi
