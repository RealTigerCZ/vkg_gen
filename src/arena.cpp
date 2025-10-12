/**
 * @file arena.cpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief Implementation of Arena allocator
 *
 * @date Created: 12. 10. 2025
 * @date Modified: 12. 10. 2025
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#include "arena.hpp"

using namespace vkg_gen;

void* Arena::allocate(size_t size, size_t alignment) {
    size_t space = remaining;
    void* ptr = current;

    // Align
    std::uintptr_t aligned = (reinterpret_cast<std::uintptr_t>(ptr) + (alignment - 1)) & ~(alignment - 1);
    size_t padding = aligned - reinterpret_cast<std::uintptr_t>(ptr);

    if (padding + size > space) {
        allocateBlock(std::max(DEFAULT_BLOCK_SIZE, size + alignment));
        return allocate(size, alignment); // try again in new block
    }

    current = reinterpret_cast<char*>(aligned + size);
    remaining = space - (padding + size);
    return reinterpret_cast<void*>(aligned);
}


void Arena::reset() noexcept {
    current = nullptr;
    remaining = 0;
    blocks.clear();
}


void Arena::allocateBlock(size_t size) {
    auto block = std::make_unique<char[]>(size);
    current = block.get();
    remaining = size;
    blocks.push_back(std::move(block));
}
