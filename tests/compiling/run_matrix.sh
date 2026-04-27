#!/bin/bash
# This script file was generated using Google Gemini

set -e

TEST_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$TEST_DIR/../.." && pwd)"

# --- Color Definitions ---
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}🚀 Starting Smart Parallel Matrix Test from: $TEST_DIR${NC}"

docker run --rm -v "$ROOT_DIR:/workspace" -w /workspace/tests/compiling vkg_gen_test_env bash -c '
    set -e

    RED="\033[0;31m"
    GREEN="\033[0;32m"
    YELLOW="\033[1;33m"
    CYAN="\033[0;36m"
    NC="\033[0m"

    COMPILERS=(
        "g++-13" "g++-14" "g++-15"
        "clang++-18" "clang++-19" "clang++-20" "clang++-21"
        "aarch64-linux-gnu-g++" "clang++-aarch64"
    )

    # --- 🧠 SMART RESOURCE CALCULATION ---
    CORES=$(lscpu -b -p=Core,Socket 2>/dev/null | grep -v "^#" | sort -u | wc -l || nproc)
    RAM_GB=$(awk "/MemTotal/ {printf \"%.0f\", \$2/1000/1000}" /proc/meminfo)

    RAM_LIMIT=$(( RAM_GB / 4 ))

    SYSTEM_CAPACITY=$CORES
    if [ "$RAM_LIMIT" -lt "$SYSTEM_CAPACITY" ]; then SYSTEM_CAPACITY=$RAM_LIMIT; fi
    if [ "$SYSTEM_CAPACITY" -lt 1 ]; then SYSTEM_CAPACITY=1; fi

    NUM_COMPILERS=${#COMPILERS[@]}

    # 1. Limit how many actual compilers run at the exact same time
    MAX_CONCURRENT_BUILDS=$SYSTEM_CAPACITY
    if [ "$MAX_CONCURRENT_BUILDS" -gt "$NUM_COMPILERS" ]; then
        MAX_CONCURRENT_BUILDS=$NUM_COMPILERS
    fi

    # 2. Divide remaining capacity so Ninja can use the spare cores safely
    NINJA_JOBS=$(( SYSTEM_CAPACITY * 2 / MAX_CONCURRENT_BUILDS ))
    if [ "$NINJA_JOBS" -lt 1 ]; then NINJA_JOBS=1; fi

    echo -e "${CYAN}📊 System Stats: ${CORES} Physical Cores, ${RAM_GB}GB RAM.${NC}"
    echo -e "${CYAN}⚙️  System Capacity: ${SYSTEM_CAPACITY} total safe units.${NC}"
    echo -e "${CYAN}🚧 Allowing max ${YELLOW}${MAX_CONCURRENT_BUILDS} concurrent compilers${CYAN} (${NINJA_JOBS} job(s) each).${NC}"
    echo "---------------------------------------------------"

    mkdir -p logs
    rm -f logs/*.log

    declare -A PIDS

    for CXX_BIN in "${COMPILERS[@]}"; do

        ACTUAL_CXX=$CXX_BIN
        CMAKE_TARGET_FLAGS=""

        if [[ "$CXX_BIN" == "clang++-aarch64" ]]; then
            ACTUAL_CXX="clang++-20"
            CMAKE_TARGET_FLAGS="-DCMAKE_CXX_COMPILER_TARGET=aarch64-linux-gnu"
        fi

        if ! command -v $ACTUAL_CXX &> /dev/null; then
            echo -e "${YELLOW}⚠️ Compiler $ACTUAL_CXX not found. Skipping $CXX_BIN.${NC}"
            continue
        fi

        # --- 🚥 THE BOTTLENECK / QUEUE ---
        # If we have reached our max concurrent builds, pause here until one finishes
        while [ $(jobs -r -p | wc -l) -ge $MAX_CONCURRENT_BUILDS ]; do
            sleep 1
        done

        (
            set -e

            BUILD_DIR="build_${CXX_BIN}"
            rm -rf $BUILD_DIR

            CXX=$ACTUAL_CXX cmake -G Ninja -S /workspace -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Release \
                -DCMAKE_COLOR_DIAGNOSTICS=ON \
                -DCMAKE_CXX_FLAGS="-fdiagnostics-color=always" \
                $CMAKE_TARGET_FLAGS

            ninja -C $BUILD_DIR -j $NINJA_JOBS

            cp /workspace/ref/vk.xml $BUILD_DIR/
            cd $BUILD_DIR

            if [[ "$CXX_BIN" == *"aarch64"* ]]; then
                qemu-aarch64 -L /usr/aarch64-linux-gnu/ ./vkgen
            else
                ./vkgen
            fi

            diff -q vkg.hpp ../golden/vkg.hpp || exit 1
            diff -q vkg.cpp ../golden/vkg.cpp || exit 1
            diff -q vkg.cppm ../golden/vkg.cppm || exit 1

            echo -e "${CYAN}🧪 Validating compilation of generated files...${NC}"

            # Copy the test program into CMAKE_BINARY_DIR
            cp ../golden/vkg_comp.cpp .

            # Trigger the custom CMake target
            ninja vkg_comp || exit 1

            echo -e "${YELLOW}⚠️ Skipping running out_comp for $CXX_BIN.${NC} because loading vulkan in docker doesnt work right now!"

            echo -e "${GREEN}✅ $CXX_BIN Passed!${NC}"

        ) > "logs/${CXX_BIN}.log" 2>&1 &

        # Record the background process ID so we can evaluate it later
        PIDS[$CXX_BIN]=$!
    done

    # Wait for all remaining queued tasks to finish
    echo -e "${CYAN}⏳ Waiting for remaining compilers to finish...${NC}"
    echo "---------------------------------------------------"

    set +e

    FAIL=0
    for CXX_BIN in "${!PIDS[@]}"; do
        wait ${PIDS[$CXX_BIN]}
        EXIT_STATUS=$?   # Capture the exit code immediately

        if [ $EXIT_STATUS -eq 0 ]; then
            echo -e "${GREEN}✅ $CXX_BIN Passed!${NC}"
        else
            echo -e "${RED}❌ $CXX_BIN Failed! Check logs/${CXX_BIN}.log${NC}"

            # Optional: Print the last 15 lines of the log so you dont even
            # have to open the file to see what failed!
            echo -e "${YELLOW}--- Last 15 lines of ${CXX_BIN}.log ---${NC}"
            tail -n 15 "logs/${CXX_BIN}.log"
            echo -e "${YELLOW}---------------------------------------${NC}"

            FAIL=1
        fi
    done

    echo "---------------------------------------------------"
    if [ $FAIL -ne 0 ]; then
        echo -e "${RED}💥 Matrix test failed! See logs above.${NC}"
        exit 1
    else
        echo -e "${GREEN}🎉 All parallel tests passed perfectly!${NC}"
    fi
'
