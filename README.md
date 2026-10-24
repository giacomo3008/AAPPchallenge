# Skip List Challenge

Apri la cartella `skip_list_challenge` in VS Code e usa il terminale in quella cartella.
Esegui i blocchi qui sotto nell'ordine. I comandi sono per macOS o Linux; su Windows usa WSL.

## 1. Preparazione, una sola volta

Servono un compilatore C++17, Python 3.10 o successivo, Google Benchmark e Matplotlib.

Su macOS con Homebrew:
```bash
brew install google-benchmark pkgconf
```

Su Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential libbenchmark-dev pkg-config
```

Per i grafici:
```bash
python3 -m pip install matplotlib==3.10.8
```

## 2. Avvia l'esempio

```bash
c++ -std=c++17 -O2 -DNDEBUG -Wall -Wextra -Wpedantic main.cpp skip_list.cpp -o demo
./demo
```

Guarda nel terminale la lista iniziale, le ricerche, lo split e l'unione.

## 3. Controlla che funzioni

```bash
c++ -std=c++17 -O2 -DNDEBUG -Wall -Wextra -Wpedantic tests.cpp skip_list.cpp -o tests
./tests
```

Deve comparire **TUTTI I TEST SUPERATI**.

Per controllare anche gli errori di memoria:
```bash
c++ -std=c++17 -g -O1 -fno-omit-frame-pointer -fsanitize=address,undefined tests.cpp skip_list.cpp -o tests_sanitized
./tests_sanitized
```

Deve terminare senza errori dei sanitizer.

## 4. Esegui gli esperimenti

Compila i due programmi separati: il primo misura i tempi, il secondo conta il lavoro.
```bash
c++ -std=c++17 -O2 -DNDEBUG -Wall -Wextra -Wpedantic benchmarks.cpp skip_list.cpp $(pkg-config --cflags --libs benchmark) -pthread -o benchmarks
c++ -std=c++17 -O2 -DNDEBUG -Wall -Wextra -Wpedantic -DCOUNT_WORK work_counts.cpp skip_list.cpp -o work_counts
```

Poi avviali:
```bash
mkdir -p results
./work_counts results/work.csv
./benchmarks --benchmark_out=results/timings.json --benchmark_out_format=json --benchmark_color=false > results/benchmark_console.txt 2>&1
```

Le misure dei tempi possono richiedere qualche minuto. Non usare `-DCOUNT_WORK` nel programma dei tempi.
In alternativa ai due avvii puoi usare `python3 run_experiments.py`.

I file `.hpp` non si avviano: sono inclusi dai sorgenti. Il Makefile e' incluso per la consegna;
usarlo e' facoltativo: `make` compila i quattro programmi, `make tests_sanitized` compila il controllo di memoria.

## 5. Crea i grafici e guarda i risultati

```bash
python3 plot_results.py
```

Questo comando legge i risultati salvati, senza ripetere gli esperimenti.

| File in `results` | Cosa guardare |
|---|---|
| `timings.png` | Come crescono i tempi con N; nel pannello split confronta versione strutturale e ricostruita. |
| `logical_work.png` | Come cresce il lavoro contato, indipendentemente dai tempi. |
| `complexity_fits.csv` | Modello stimato, coefficiente e RMS: un RMS piu' basso indica un fit piu' aderente ai dati. |
| `work.csv` | I singoli contatori, comprese allocazioni e liberazioni dei nodi. |
| `benchmark_console.txt` | Output completo e messaggi del programma dei tempi. |

I risultati inclusi sono gia' consultabili. Se rifai le prove, vengono sovrascritti.
Conserva anche `timings.json`: contiene i dati grezzi richiesti dalla consegna.

## Dati richiesti dalla consegna

- **Parametri:** N = 1024, 4096, 16384, 65536, 262144, 1048576; seed delle altezze 11, 42, 2026;
  per B si aggiunge 1000003. Query con seed 20261023 + N. Tre ripetizioni per seed, un solo thread.
  Test: 1000 turni per seed. Tempi e coefficienti sono divisi per il batch: 256 ricerche o 64 inserimenti.
- **Versioni delle misure incluse:** GCC 13.3.0, Google Benchmark 1.9.4, Python 3.12.14,
  Matplotlib 3.10.8; Linux x86_64, AMD EPYC 9V74. Flag nei comandi sopra; dettagli in `results/environment.json`.
  Se rifai le misure, aggiorna ambiente e versioni con quelli del tuo computer
  (`c++ --version` e `pkg-config --modversion benchmark`).
- **Memoria:** test e ASan/UBSan superati; log in `results/tests.txt` e `results/sanitizers.txt`.
  Nel contenitore LeakSanitizer non e' disponibile: e' stato usato
  `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 ./tests_sanitized`.
- **Conteggi:** confronti fra chiavi; passi di scansione verso nodi dati; scritture di `next`, incluse
  inizializzazioni e sentinelle. Allocazioni/liberazioni contano solo i nodi dati. Il grafico somma i primi tre.
- **Limiti:** tre seed, ambiente condiviso, inserimenti dopo il massimo e altezza limitata a 32.
  Un fit dei tempi va confrontato con l'analisi teorica.
- **Fonti e strumenti:** consegna del corso, [Google Benchmark](https://github.com/google/benchmark/tree/v1.9.4)
  e assistenza di ChatGPT per codice, test, script e documentazione.

Prima della consegna aggiungi `report.pdf` in inglese (massimo due A4) e compila:
**Autore: DA COMPILARE. Matricola: DA COMPILARE.**

Dichiarazione da confermare dall'autore: «Il lavoro e' individuale; dichiaro le fonti e
l'assistenza AI indicate e confermo di aver verificato e compreso codice e risultati».
