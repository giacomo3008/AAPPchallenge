#include "skip_list.hpp"

#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    // Il seed e' facoltativo: ./demo 11. Senza argomenti usiamo 42.
    std::uint64_t seed = argc > 1 ? std::stoull(argv[1]) : 42;
    SkipList list(seed);

    for (std::int64_t key : {30, 10, 20, 40}) {
        list.insert(key);
    }

    std::cout << "Lista iniziale:\n";
    list.print();
    //std::boolalpha fa stampare i valori booleani come true e false, invece di 1 e 0.
    std::cout << std::boolalpha;
    std::cout << "search(20): " << list.search(20) << '\n';
    std::cout << "search(25): " << list.search(25) << '\n';
    std::cout << "insert(20), duplicato: " << list.insert(20) << '\n';

    auto [left, right] = list.split(25);
    std::cout << "\nParte sinistra (<= 25):\n";
    left.print();
    std::cout << "Parte destra (> 25):\n";
    right.print();

    auto joined = SkipList::ordered_union(left, right);
    std::cout << "\nListe riunite:\n";
    joined.print();
}
