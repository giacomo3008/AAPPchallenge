#pragma once

#include "skip_list.hpp"
#include <algorithm>
#include <random>
#include <vector>

// Stessi dati per il programma dei tempi e quello dei contatori.
namespace experiment {
const int sizes[] = {1024, 4096, 16384, 65536, 262144, 1048576};
const std::uint64_t seeds[] = {11, 42, 2026};
const std::uint64_t workloadSeed = 20261023;
const int searchBatch = 256;
const int insertBatch = 64;

inline void fill(SkipList& list, int n, int start = 0) {
    for (int i = 0; i < n; ++i) {
        std::int64_t key = 2LL * (start + i);
        list.insert(key);
    }
}

inline std::vector<std::int64_t> queries(int n) {
    std::mt19937_64 generator(workloadSeed + n);
    std::vector<std::int64_t> result;
    for (int i = 0; i < searchBatch / 2; ++i) {
        int index = generator() % n;
        std::int64_t present = 2LL * index;
        result.push_back(present);

        index = generator() % n;
        std::int64_t absent = 2LL * index + 1;
        result.push_back(absent);
    }
    std::shuffle(result.begin(), result.end(), generator);
    return result;
}

inline std::uint64_t secondSeed(std::uint64_t seed) {
    return seed + 1000003;
}
}
