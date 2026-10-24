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

    //const per dire che questa funzione non cambia l'oggetto sul quale chiami il metodo
    //puoi chiamare questa funzione anche su una skipList const
    bool search(std::int64_t key) const;
    bool insert(std::int64_t key);

    // Consuma A e B: diventano vuote e riutilizzabili.
    // Richiede oggetti distinti e tutte le chiavi di A < tutte quelle di B.
    static SkipList ordered_union(SkipList& a, SkipList& b);

    // Consuma questa lista e restituisce {chiavi <= x, chiavi > x}.
    std::pair<SkipList, SkipList> split(std::int64_t x);

    // Versione di confronto: ricrea i nodi usando insert e libera gli originali.
    std::pair<SkipList, SkipList> split_rebuild(std::int64_t x);

    // Questi metodi servono solo ai test, mai dentro le misure dei tempi.
    std::vector<std::int64_t> values() const;
    // Controlla ordine, altezze e collegamenti; richiesto dalla consegna.
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

    // Attivi solo compilando skip_list.cpp con -DCOUNT_WORK.
    // Le allocazioni/liberazioni contano i nodi dati, non le sentinelle.
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

    /*Il primo viene usato quando costruiamo una lista trasferendo il contenuto di un’altra. 
    Il secondo serve quando la lista di destinazione esiste già.
    Usati da Union e Split*/
    SkipList(SkipList&& other);
    SkipList& operator=(SkipList&& other);
    /*
    SkipList risultato;
        //risultato esiste già: qui avviene un'assegnazione
        rultato = SkipList::ordered_union(a, b);
        */

private:
    static constexpr int MAX_LEVEL = 32;

    struct Node {
        std::int64_t key;
        std::vector<Node*> next;

        Node(std::int64_t value, int height)
            : key(value), next(height, nullptr) {}
        /*dodpo i : vengono inizializzati i campi prima di eseguire il corpo del costruttore.
        key(value) inizializza key con il valore ricevuto.
        next(height, nullptr) costruisce il vettore next con height elementi, tutti inizializzati a nullptr.*/

        /*new Node(20, 3); dove next.size sarà 3, next[0],[1],[2] saranno nullptr 
        Abbiamo creato un nodo con tre puntatori, uno per ciascuno dei suoi livelli.
        nullptr significa che quel puntatore, per il momento, non indica nessun nodo. 
        Sarà insert a collegarlo ai nodi successivi.*/
    };

    Node* head;          
    /*head è il puntatore; il nodo sentinella è l’oggetto a cui punta.  Quindi gli assegniamo:
        - chiave 0;
        - MAX_LEVEL puntatori, uno per ogni possibile livello.
    Quello 0 è un valore di riempimento: il nostro algoritmo non legge mai la chiave della 
    sentinella. Per questo non serve rappresentare davvero −∞*/
    int levels;             // Numero di livelli attivi; almeno 1 anche se vuota.
    std::mt19937_64 random;  // Questo generatore decide soltanto le altezze.
    static Counters counters;

    int randomHeight();
    std::array<Node*, MAX_LEVEL> findPredecessors(std::int64_t key,
                                                bool includeEqual) const;
    /*Questa funzione cerca il nodo che precede la posizione cercata a ciascun livello. 
    se ho livello 0: 10,20,30,40 e livello 1: 10,30,   con findPredecessor(25,false) mi da:
    previous[0] che sarebbe il nodo con chiave 20
    previous[1] che sarebbe il nodo con chiave 10
    se non esiste una chiave precedente, viene restituita la sentinella HEAD
    
    includeEqual decide se fermarsi a <key(FALSE) oppure <=key (TRUE)
    
    
    Questa funzione aiuta diverse operazioni:
    - search controlla il nodo successivo al predecessore;
    - insert collega il nuovo nodo dopo i predecessori;
    - split individua dove tagliare i collegamenti.*/  

    /*Azzera i collegamenti della sentinella e riporta levels a 1. senza distruggere i nodi*/
    void resetHead();  
    /*resetHead() serve quando i nodi sono già stati trasferiti a un’altra lista.
    Per esempio, dopo ordered_union(a, b), i nodi appartengono alla lista risultante. 
    Dobbiamo svuotare le teste di a e b, lasciando vivi quei nodi.*/
    
    /*Percorre e distrugge tutti i nodi, poi chiama resetHead()*/
    void clear();   
};
