# Skip List Challenge

Open the terminal in the project folder. These commands are for macOS and Linux.
On Windows, you can use WSL.

**Author:** Giacomo Rigo. **Student ID:** 308362.

## 1. Setup

You need a compiler that supports C++17, Google Benchmark, pkg-config,
Python 3.10 or later, and Matplotlib.

On macOS with Homebrew:
```bash
brew install google-benchmark pkgconf
```

On Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential libbenchmark-dev pkg-config
```

For the plots:
```bash
python3 -m pip install matplotlib==3.10.8
```

## 2. Compilation

You can use Make or the manual commands. Choose one method.

With Make:
```bash
make
make tests_sanitized
```

`make` builds `demo`, `tests`, `benchmarks`, and `work_counts`.
The second command builds the tests with memory checks.

Or compile manually:
```bash
c++ -std=c++17 -O2 -DNDEBUG -Wall -Wextra -Wpedantic main.cpp skip_list.cpp -o demo
c++ -std=c++17 -O2 -DNDEBUG -Wall -Wextra -Wpedantic tests.cpp skip_list.cpp -o tests
c++ -std=c++17 -g -O1 -fno-omit-frame-pointer -fsanitize=address,undefined tests.cpp skip_list.cpp -o tests_sanitized
c++ -std=c++17 -O2 -DNDEBUG -Wall -Wextra -Wpedantic benchmarks.cpp skip_list.cpp $(pkg-config --cflags --libs benchmark) -pthread -o benchmarks
c++ -std=c++17 -O2 -DNDEBUG -Wall -Wextra -Wpedantic -DCOUNT_WORK work_counts.cpp skip_list.cpp -o work_counts
```

`COUNT_WORK` enables the counters. Use it only for `work_counts`, not for timing.
The Makefile already handles this.

On my Mac, I needed to select SDK 26.5. In this case, I use:
```bash
make clean
make CXX="/usr/bin/clang++ -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX26.5.sdk -isystem /Library/Developer/CommandLineTools/SDKs/MacOSX26.5.sdk/usr/include/c++/v1" all tests_sanitized
```

## 3. Example and tests

```bash
./demo
./tests
./tests_sanitized
```

The demo shows insertions, searches, split, and union.
Both test programs should print `TUTTI I TEST SUPERATI` (all tests passed).
The sanitizer version should finish without reporting errors.
With Make, `make test` builds the tests if needed and runs the same tests as `./tests`.

The tests compare the list with `std::set` and check its structure. They cover
empty lists, duplicate keys, boundary keys, reuse of inputs, and preservation of nodes.
They run 1000 mixed operations for each seed: 11, 42, and 2026.
The final check on Linux with GCC 14.2.0 passed all tests and ASan/UBSan checks,
with leak detection enabled. Logs are in `results/tests.txt` and
`results/sanitizers.txt`. The timing results are from the Mac described in section 6.

## 4. Experiments and plots

```bash
mkdir -p results
./work_counts results/work.csv
./benchmarks --benchmark_out=results/timings.json --benchmark_out_format=json --benchmark_color=false > results/benchmark_console.txt 2>&1
python3 plot_results.py
```

`work_counts` counts the work done by the algorithm. `benchmarks` measures the times.
The measurements may take a few minutes. Previous results are overwritten.
To create the plots again without running the experiments, use `python3 plot_results.py`.

The following files are saved in `results`:

- `timings.json`: raw timing data, to keep for the submission;
- `benchmark_console.txt`: benchmark output;
- `work.csv`: work counts and node allocation/free counts;
- `timings.png` and `logical_work.png`: plots, including the comparison of the two split methods;
- `complexity_fits.csv`: automatically selected model, coefficient, and RMS error as a percentage.

## 5. Experiment settings

The sizes are N = 1024, 4096, 16384, 65536, 262144, and 1048576.
The height seeds are 11, 42, and 2026. The second list in the union uses
seed + 1000003. Each measurement is repeated three times per seed, on one thread.

Search queries use 50% present keys and 50% missing keys. Keys are sampled uniformly,
with repeats allowed, and the queries are then shuffled. The query generator is
separate from the height generator and uses seed 20261023 + N.
Insertions add new keys above the current maximum: each batch grows the list
from N to N+64 keys. Union uses two lists with N/2 keys each.
Split divides the list into two equal halves.

Each iteration measures 256 searches or 64 insertions. For the other operations,
it measures one call. Plotted times and fitted coefficients are divided by the batch size.
The iteration counts are limited to 256 for search/insert, 1000 for union/split,
and 3 for the rebuilding split, to keep the experiments short.
Input setup and restoration are not timed.

The counters record key comparisons, steps to data nodes, and writes to the
`next` pointers, including initialization and sentinels. The work plot adds
these three counts. Allocation and free counts include only data nodes.
The rebuilding split uses normal insertions and frees the original nodes.

## 6. Mac environment

Apple Clang 21.0.0 (clang-2100.1.1.101), Google Benchmark 1.10.0,
Python 3.13.7, and Matplotlib 3.10.8; macOS 26.6.2, arm64, Apple M2.
The code is compiled as C++17 with macOS SDK 26.5, using the paths shown above.
The optimized build flags are `-std=c++17 -O2 -DNDEBUG -Wall -Wextra -Wpedantic`,
with `-pthread` for the benchmarks. The sanitizer flags are shown in section 2.

## 7. Limits, sources

The experiments use three seeds, heights capped at 32 for the listed sizes,
and insertions above the maximum key. The automatic fits describe these measurements;
they do not prove the theoretical complexity. The analysis and comparison with the
work counts belong in the report.

Sources and tools: the assignment and course lectures,
[Google Benchmark and its official guide](https://github.com/google/benchmark),
and help from ChatGPT with code, tests, scripts, documentation, and the report.
