#include "skip_list.hpp"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <set>

using Key = std::int64_t;
using Reference = std::set<Key>;

// Un normale if: rimane attivo anche compilando con -DNDEBUG.
void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "TEST FALLITO: " << message << '\n';
        std::exit(1);
    }
}

void checkList(const SkipList& list, const Reference& expected) {
    check(list.validate(), "Struttura della lista non valida");
    std::vector<Key> values;
    for (Key key : expected) values.push_back(key);
    check(list.values() == values, "Contenuto diverso da std::set");
}

// Accoda le schede dei nodi, senza modificare le due liste.
std::vector<SkipList::NodeInfo> combined(const SkipList& a, const SkipList& b) {
    auto nodes = a.debugNodes();
    for (auto node : b.debugNodes()) nodes.push_back(node);
    return nodes;
}

// Divide e riunisce: alla fine list deve essere uguale a prima.
void splitAndJoin(SkipList& list, const Reference& expected, Key pivot) {
    auto before = list.debugNodes();
    Reference expectedLeft, expectedRight;
    for (Key key : expected) {
        if (key <= pivot) expectedLeft.insert(key);
        else expectedRight.insert(key);
    }

    auto parts = list.split(pivot);
    checkList(list, {});
    checkList(parts.first, expectedLeft);
    checkList(parts.second, expectedRight);
    check(combined(parts.first, parts.second) == before, "Split cambia i nodi");

    list = SkipList::ordered_union(parts.first, parts.second);
    checkList(parts.first, {});
    checkList(parts.second, {});
    checkList(list, expected);
    check(list.debugNodes() == before, "Unione cambia i nodi");
}

void basicTests() {
    Key minimum = std::numeric_limits<Key>::min();
    Key maximum = std::numeric_limits<Key>::max();
    std::vector<Reference> cases = {
        {}, {0}, {10, 20, 30, 40}, {minimum, -1, 0, 1, maximum}
    };

    for (const Reference& expected : cases) {
        SkipList list(42);
        for (Key key : expected) check(list.insert(key), "Insert fallito");
        checkList(list, expected);

        auto before = list.debugNodes();
        for (Key key : expected) check(!list.insert(key), "Duplicato inserito");
        check(list.debugNodes() == before, "Duplicato cambia i nodi");
        for (Key key : {minimum, Key(-10), Key(0), Key(20), Key(25), maximum}) {
            bool found = list.search(key);
            check(found == (expected.count(key) == 1), "Search errato");
        }
        check(list.debugNodes() == before, "Search modifica i nodi");

        for (Key pivot : {minimum, Key(0), Key(20), Key(25), maximum}) {
            splitAndJoin(list, expected, pivot);

            // Anche la versione ricostruita deve dividere correttamente.
            Reference left, right;
            for (Key key : expected) {
                if (key <= pivot) left.insert(key);
                else right.insert(key);
            }
            auto rebuilt = list.split_rebuild(pivot);
            checkList(list, {});
            checkList(rebuilt.first, left);
            checkList(rebuilt.second, right);
            list.insert(123);
            checkList(list, {123});
            checkList(rebuilt.first, left);
            checkList(rebuilt.second, right);
            list = SkipList::ordered_union(rebuilt.first, rebuilt.second);
            checkList(list, expected);
        }
    }
    std::cout << "Insert, search, split e baseline: OK\n";
}

void unionTests() {
    for (int n : {0, 1, 1000}) {
        for (int m : {0, 1, 1000}) {
            SkipList a(11), b(1000014);
            Reference expected;
            for (int i = 0; i < n; ++i) {
                a.insert(i - 2000);
                expected.insert(i - 2000);
            }
            for (int i = 0; i < m; ++i) {
                b.insert(i + 1);
                expected.insert(i + 1);
            }
            auto before = combined(a, b);
            auto result = SkipList::ordered_union(a, b);
            checkList(result, expected);
            check(result.debugNodes() == before, "Unione cambia i nodi");
            checkList(a, {});
            checkList(b, {});
        }
    }
    std::cout << "Unioni con liste vuote o di dimensioni diverse: OK\n";
}

void independenceTests() {
    SkipList list(11);
    for (Key key : {10, 20, 30, 40}) list.insert(key);
    auto parts = list.split(25);

    // Riutilizzo l'input e modifico le due uscite, una alla volta.
    list.insert(5);
    checkList(parts.first, {10, 20});
    checkList(parts.second, {30, 40});
    parts.first.insert(15);
    checkList(parts.second, {30, 40});
    parts.second.insert(35);
    checkList(parts.first, {10, 15, 20});
    checkList(parts.second, {30, 35, 40});
    checkList(list, {5});

    auto result = SkipList::ordered_union(parts.first, parts.second);
    checkList(parts.first, {});
    checkList(parts.second, {});
    parts.first.insert(-5);
    parts.second.insert(50);
    checkList(parts.first, {-5});
    checkList(parts.second, {50});
    checkList(result, {10, 15, 20, 30, 35, 40});
    std::cout << "Indipendenza delle liste e riuso degli input: OK\n";
}

void mixedTests(std::uint64_t seed) {
    SkipList list(seed);
    Reference expected;
    std::mt19937_64 workload(seed + 10000); // Non decide le altezze.
    int roundTrips = 0;

    for (int i = 0; i < 1000; ++i) {
        Key key = static_cast<Key>(workload() % 1001) - 500;
        int operation = workload() % 3;
        if (operation == 0) {
            auto before = list.debugNodes();
            check(list.search(key) == (expected.count(key) == 1), "Search casuale errato");
            check(list.debugNodes() == before, "Search modifica i nodi");
        } else if (operation == 1) {
            auto before = list.debugNodes();
            bool isNew = (expected.count(key) == 0);
            check(list.insert(key) == isNew, "Insert casuale errato");
            expected.insert(key);
            if (!isNew) check(list.debugNodes() == before, "Duplicato cambia i nodi");
        } else {
            splitAndJoin(list, expected, key);
            ++roundTrips;
        }
        checkList(list, expected);
    }
    std::cout << "Seed " << seed << ": 1000 turni, " << roundTrips
              << " sequenze split/unione: OK\n";
}

int main() {
    basicTests();
    unionTests();
    independenceTests();
    for (std::uint64_t seed : {11, 42, 2026}) mixedTests(seed);
    std::cout << "TUTTI I TEST SUPERATI\n";
}
