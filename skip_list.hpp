#pragma once

#include <array>
#include <cstdint>
#include <random>
#include <utility>
#include <vector>

class SkipList {
public:
    explicit SkipList(std::uint64_t seed = 42);
    ~SkipList();

    bool search(std::int64_t key) const;
    bool insert(std::int64_t key);

    static SkipList ordered_union(SkipList& a, SkipList& b);

    std::pair<SkipList, SkipList> split(std::int64_t x);

    std::pair<SkipList, SkipList> split_rebuild(std::int64_t x);

    std::vector<std::int64_t> values() const;
    bool validate() const;

    struct NodeInfo {
        std::int64_t key;
        const void* address;
        int height;

        bool operator==(const NodeInfo& other) const {
            return key == other.key && address == other.address && height == other.height;
        }
    };
    std::vector<NodeInfo> debugNodes() const;

    struct Counters {
        std::uint64_t comparisons = 0;
        std::uint64_t traversals = 0;
        std::uint64_t link_updates = 0;
        std::uint64_t data_allocations = 0;
        std::uint64_t data_frees = 0;
    };
    static void resetCounters();
    static Counters getCounters();

    void print() const;

    SkipList(SkipList&& other);
    SkipList& operator=(SkipList&& other);


private:
    static constexpr int MAX_LEVEL = 32;

    struct Node {
        std::int64_t key;
        std::vector<Node*> next;

        Node(std::int64_t value, int height)
            : key(value), next(height, nullptr) {}
    };

    Node* head;          
    int levels;             // Numero di livelli attivi; almeno 1 anche se vuota.
    std::mt19937_64 random;  // Questo generatore decide soltanto le altezze.
    static Counters counters;

    int randomHeight();
    std::array<Node*, MAX_LEVEL> findPredecessors(std::int64_t key,
                                                bool includeEqual) const;
    void resetHead();  

    void clear();   
};
