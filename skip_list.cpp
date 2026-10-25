#include "skip_list.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

// Nel programma dei tempi COUNT non esegue nessuna istruzione.
#ifdef COUNT_WORK
#define COUNT(field, amount) (counters.field += (amount))
#else
#define COUNT(field, amount) ((void)0)
#endif

SkipList::Counters SkipList::counters;

void SkipList::resetCounters() {
    counters = {};
}

SkipList::Counters SkipList::getCounters() {
    return counters;
}

SkipList::SkipList(std::uint64_t seed)
    : head(new Node(0, MAX_LEVEL)), levels(1), random(seed) {
    COUNT(link_updates, MAX_LEVEL);
}

SkipList::~SkipList() {
    clear();
    delete head;
}

int SkipList::randomHeight() {
    int height = 1;

    // Ogni numero pari equivale a "testa": promozione con probabilita 1/2.
    while (height < MAX_LEVEL && (random() % 2 == 0)) {
        ++height;
    }

    return height;
}

std::array<SkipList::Node*, SkipList::MAX_LEVEL>
SkipList::findPredecessors(std::int64_t key, bool includeEqual) const {
    std::array<Node*, MAX_LEVEL> previous;
    for (int i = 0; i < MAX_LEVEL; i++) {
        previous[i] = head;
    }
    Node* current = head;

    // Scendiamo dall'alto, ricordando dove ci fermiamo a ciascun livello.
    for (int level = levels - 1; level >= 0; --level) {
        while (current->next[level] != nullptr) {
            std::int64_t nextKey = current->next[level]->key;
            COUNT(comparisons, 1);
            bool canAdvance = includeEqual ? nextKey <= key : nextKey < key;

            if (!canAdvance) break;
            current = current->next[level];
            COUNT(traversals, 1);
        }

        previous[level] = current;
    }

    return previous;
}

bool SkipList::search(std::int64_t key) const {
    auto previous = findPredecessors(key, false);
    Node* candidate = previous[0]->next[0];

    if (candidate != nullptr) COUNT(comparisons, 1);
    return candidate != nullptr && candidate->key == key;
}

bool SkipList::insert(std::int64_t key) {
    auto previous = findPredecessors(key, false);
    Node* candidate = previous[0]->next[0];

    // Un duplicato non cambia la struttura e non estrae una nuova altezza.
    if (candidate != nullptr) COUNT(comparisons, 1);
    if (candidate != nullptr && candidate->key == key) return false;

    int height = randomHeight();
    Node* node = new Node(key, height);
    COUNT(data_allocations, 1);
    COUNT(link_updates, height); // Inizializzazione dei next a nullptr.

    for (int level = 0; level < height; ++level) {
        node->next[level] = previous[level]->next[level];
        previous[level]->next[level] = node;
        COUNT(link_updates, 2);
    }

    levels = std::max(levels, height);
    return true;
}

SkipList SkipList::ordered_union(SkipList& a, SkipList& b) {
    if (&a == &b) {
        throw std::invalid_argument("L'unione richiede due liste distinte");
    }

    // Controlliamo l'ordine soltanto se entrambe le liste hanno nodi.
    if (a.head->next[0] != nullptr && b.head->next[0] != nullptr) {
        Node* lastA = a.head;

        // Troviamo il nodo con la chiave massima di a.
        for (int level = a.levels - 1; level >= 0; --level) {
            while (lastA->next[level] != nullptr) {
                lastA = lastA->next[level];
                COUNT(traversals, 1);
            }
        }

        Node* firstB = b.head->next[0];

        COUNT(comparisons, 1);
        if (lastA->key >= firstB->key) {
            throw std::invalid_argument(
                "Tutte le chiavi di a devono essere minori di quelle di b"
            );
        }
    }

    SkipList result(a.random());
    result.levels = std::max(a.levels, b.levels);
    Node* current = a.head;

    // Troviamo le estremita di A con un'unica discesa dall'alto.
    for (int level = result.levels - 1; level >= 0; --level) {
        while (current->next[level] != nullptr) {
            current = current->next[level];
            COUNT(traversals, 1);
        }

        if (a.head->next[level] != nullptr) {
            result.head->next[level] = a.head->next[level];
            current->next[level] = b.head->next[level];
            COUNT(link_updates, 2);
        } else {
            result.head->next[level] = b.head->next[level];
            COUNT(link_updates, 1);
        }
    }

    // I nodi appartengono ora a result: non dobbiamo distruggerli.
    a.resetHead();
    b.resetHead();
    return result;
}

std::pair<SkipList, SkipList> SkipList::split(std::int64_t x) {
    auto previous = findPredecessors(x, true);
    SkipList left(random());
    SkipList right(random());
    left.levels = right.levels = levels;

    for (int level = 0; level < levels; ++level) {
        // Il successore dell'ultimo nodo <= x e' l'inizio della parte destra.
        right.head->next[level] = previous[level]->next[level];
        previous[level]->next[level] = nullptr;

        // Se il predecessore era head, il collegamento appena tagliato e' nullo:
        // significa che a questo livello non esistono nodi della parte sinistra.
        left.head->next[level] = head->next[level];
        COUNT(link_updates, 3);
    }

    /*I due while eliminano dal conteggio i livelli superiori rimasti vuoti
    Quindi entrambe le nuove liste hanno provvisoriamente lo stesso numero di 
    livelli della lista originale.
    Dopo la separazione, però, alcune potrebbero avere livelli superiori vuoti.*/
    while (left.levels > 1 && left.head->next[left.levels - 1] == nullptr) {
        --left.levels;
    }
    while (right.levels > 1 && right.head->next[right.levels - 1] == nullptr) {
        --right.levels;
    }

    resetHead();
    return {std::move(left), std::move(right)};
}

std::pair<SkipList, SkipList> SkipList::split_rebuild(std::int64_t x) {
    SkipList left(random());
    SkipList right(random());

    for (Node* node = head->next[0]; node != nullptr; node = node->next[0]) {
        COUNT(traversals, 1);
        COUNT(comparisons, 1);
        if (node->key <= x) left.insert(node->key);
        else right.insert(node->key);
    }

    // Fa parte dell'operazione: i vecchi nodi devono essere liberati.
    clear();
    return {std::move(left), std::move(right)};
}

void SkipList::resetHead() {
    for (Node*& pointer : head->next) {
        pointer = nullptr;
        COUNT(link_updates, 1);
    }
    levels = 1;
}

void SkipList::clear() {
    Node* current = head->next[0];

    while (current != nullptr) {
        Node* next = current->next[0];
        COUNT(traversals, 1);
        COUNT(data_frees, 1);
        delete current;
        current = next;
    }

    resetHead();
}

SkipList::SkipList(SkipList&& other) : SkipList(other.random()) {
    // Scambiamo la nuova testa vuota con quella che possiede i nodi.
    std::swap(head, other.head);
    std::swap(levels, other.levels);
    std::swap(random, other.random);
}

SkipList& SkipList::operator=(SkipList&& other) {
    if (this != &other) {
        clear();
        std::swap(head, other.head);
        std::swap(levels, other.levels);
        std::swap(random, other.random);
    }

    return *this;
}

std::vector<std::int64_t> SkipList::values() const {
    std::vector<std::int64_t> result;
    for (Node* node = head->next[0]; node != nullptr; node = node->next[0]) {
        result.push_back(node->key);
    }
    return result;
}

std::vector<SkipList::NodeInfo> SkipList::debugNodes() const {
    std::vector<NodeInfo> result;
    for (Node* node = head->next[0]; node != nullptr; node = node->next[0]) {
        result.push_back({node->key, node, static_cast<int>(node->next.size())});
    }
    return result;
}

bool SkipList::validate() const {
    if (head == nullptr || head->next.size() != MAX_LEVEL) return false;
    if (levels < 1 || levels > MAX_LEVEL) return false;

    // Il livello 0 contiene tutti i nodi, in ordine crescente.
    std::vector<Node*> bottom;
    Node* previous = nullptr;
    for (Node* node = head->next[0]; node != nullptr; node = node->next[0]) {
        if (node == head) return false;
        if (node->next.empty() || node->next.size() > MAX_LEVEL) return false;
        int height = static_cast<int>(node->next.size());
        if (height > levels) return false;
        // Anche un ciclo rompe l'ordine: si torna a una chiave minore o uguale.
        if (previous != nullptr && previous->key >= node->key) return false;
        bottom.push_back(node);
        previous = node;
    }

    if (bottom.empty() && levels != 1) return false;
    if (!bottom.empty() && head->next[levels - 1] == nullptr) return false;
    for (int level = levels; level < MAX_LEVEL; ++level) {
        if (head->next[level] != nullptr) return false;
    }

    // Nei livelli superiori devono esserci gli stessi nodi abbastanza alti.
    for (int level = 1; level < levels; ++level) {
        Node* current = head->next[level];
        for (Node* node : bottom) {
            int height = static_cast<int>(node->next.size());
            if (height > level) {
                if (current != node) return false;
                current = node->next[level];
            }
        }
        if (current != nullptr) return false;
    }
    return true;
}

void SkipList::print() const {
    for (int level = levels - 1; level >= 0; --level) {
        std::cout << "Livello " << level << ": ";

        for (Node* node = head->next[level]; node != nullptr; node = node->next[level]) {
            std::cout << node->key << ' ';
        }

        std::cout << '\n';
    }
}
