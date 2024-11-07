
#ifndef FMU4CPP_TEMPLATE_HASH_HPP
#define FMU4CPP_TEMPLATE_HASH_HPP

#include <cstdint>
#include <string>


//https://stackoverflow.com/questions/66764096/calculating-stdhash-using-different-compilers
inline uint64_t fnv1a(std::string const &text) {

    if constexpr (sizeof(void *) == 4) {
        // 32-bit environment
        uint32_t constexpr fnv_prime = 16777619U;
        uint32_t constexpr fnv_offset_basis = 2166136261U;

        uint32_t hash = fnv_offset_basis;

        for (auto c: text) {
            hash ^= c;
            hash *= fnv_prime;
        }

        return hash;
    } else {
        // 64-bit environment
        uint64_t constexpr fnv_prime = 1099511628211ULL;
        uint64_t constexpr fnv_offset_basis = 14695981039346656037ULL;

        uint64_t hash = fnv_offset_basis;

        for (auto c: text) {
            hash ^= c;
            hash *= fnv_prime;
        }

        return hash;
    }
}

#endif//FMU4CPP_TEMPLATE_HASH_HPP
