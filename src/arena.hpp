/**
 * @file arena.hpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief Simple arena memory alocator, that allocates memory in blocks, so it's more cache friendly (because xml is full of small data)
 *        and whole xml can be freed at once. This approach is not ideal if you need to change xml at runtime (its not possible to deallocate memory).
 *
 * @date Created: 12. 10. 2025
 * @date Modified: 1. 11. 2025
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#pragma once

#include <string_view>
#include <string>
#include <memory>
#include <vector>

namespace vkg_gen {
    class Arena {
        static constexpr size_t DEFAULT_BLOCK_SIZE = 64 * 1024; // 64 KB blocks
        std::vector<std::unique_ptr<char[]>> blocks;
        char* current = nullptr;
        size_t remaining = 0;

    public:
        Arena() = default;
        explicit Arena(size_t initialBlockSize) {
            allocateBlock(initialBlockSize);
        }

        void* allocate(size_t size, size_t alignment = alignof(std::max_align_t));

        template<typename T, typename... Args>
        T* make(Args&&... args) {
            void* mem = allocate(sizeof(T), alignof(T));
            return new (mem) T(std::forward<Args>(args)...);
        }

        void reset() noexcept;

        using sv = std::string_view;
        sv storeString(const char* str) { return storeString(sv(str)); }
        sv storeString(const std::string& str) { return storeString(sv(str)); }
        sv storeString(const sv& str);

    private:
        void allocateBlock(size_t size);
    };
}
