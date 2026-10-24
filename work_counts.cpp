#include "experiment.hpp"
#include <fstream>
#include <iostream>

using namespace experiment;

void writeRow(std::ostream& out, const char* operation, int n, std::uint64_t seed, int batch) {
    auto c = SkipList::getCounters();
    out << operation << ',' << n << ',' << seed << ',' << batch << ','
        << c.comparisons << ',' << c.traversals << ',' << c.link_updates << ','
        << c.data_allocations << ',' << c.data_frees << '\n';
}

int main(int argc, char** argv) {
    std::ofstream file(argc > 1 ? argv[1] : "work.csv");
    if (!file) { std::cerr << "Impossibile aprire il CSV di output\n"; return 1; }
    file << "operation,n,seed,batch,comparisons,traversals,link_updates,data_allocations,data_frees\n";
    for (auto seed : seeds) {
        for (int n : sizes) {
            {
                SkipList list(seed);
                fill(list, n);
                auto workload = queries(n);
                SkipList::resetCounters();
                for (auto key : workload) list.search(key);
                writeRow(file, "search", n, seed, searchBatch);
            }
            {
                SkipList list(seed);
                fill(list, n);
                SkipList::resetCounters();
                for (int i = 0; i < insertBatch; ++i) list.insert(2LL * (n + i));
                writeRow(file, "insert", n, seed, insertBatch);
            }
            {
                SkipList a(seed), b(secondSeed(seed));
                fill(a, n / 2);
                fill(b, n / 2, n / 2);
                SkipList::resetCounters();
                auto joined = SkipList::ordered_union(a, b);
                writeRow(file, "ordered_union", n, seed, 1);
            }
            {
                SkipList list(seed);
                fill(list, n);
                SkipList::resetCounters();
                auto parts = list.split(n - 1);
                writeRow(file, "split", n, seed, 1);
            }
            {
                SkipList list(seed);
                fill(list, n);
                SkipList::resetCounters();
                auto parts = list.split_rebuild(n - 1);
                writeRow(file, "split_rebuild", n, seed, 1);
            }
        }
        std::cout << "Conteggi completati per seed " << seed << '\n';
    }
}
