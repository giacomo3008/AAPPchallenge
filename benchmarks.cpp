#include "experiment.hpp"
#include <benchmark/benchmark.h>
#include <string>

using namespace experiment;

void BM_Search(benchmark::State& state, std::uint64_t seed) {
    int n = static_cast<int>(state.range(0));
    SkipList list(seed);
    fill(list, n);
    auto workload = queries(n);
    for (auto _ : state) {
        for (auto key : workload) {
            bool found = list.search(key);
            benchmark::DoNotOptimize(found);
        }
    }
    state.SetComplexityN(n);
    state.SetItemsProcessed(state.iterations() * searchBatch);
}

void BM_Insert(benchmark::State& state, std::uint64_t seed) {
    int n = static_cast<int>(state.range(0));
    SkipList list(seed);
    fill(list, n);
    const std::int64_t lastOriginal = 2LL * (n - 1);
    for (auto _ : state) {
        for (int i = 0; i < insertBatch; ++i) {
            // Tutte assenti e dopo le chiavi originali: ripristino semplice con split.
            bool inserted = list.insert(2LL * (n + i));
            benchmark::DoNotOptimize(inserted);
        }
        benchmark::ClobberMemory();
        state.PauseTiming();
        {
            auto parts = list.split(lastOriginal);
            list = std::move(parts.first);
        } // I 64 nodi aggiunti vengono liberati mentre il timer e' fermo.
        state.ResumeTiming();
    }
    state.SetComplexityN(n);
    state.SetItemsProcessed(state.iterations() * insertBatch);
}

void BM_Union(benchmark::State& state, std::uint64_t seed) {
    int n = static_cast<int>(state.range(0));
    SkipList a(seed), b(secondSeed(seed));
    fill(a, n / 2);
    fill(b, n / 2, n / 2);
    for (auto _ : state) {
        {
            auto joined = SkipList::ordered_union(a, b);
            benchmark::DoNotOptimize(joined);
            benchmark::ClobberMemory();
            state.PauseTiming();
            auto parts = joined.split(n - 2);
            a = std::move(parts.first);
            b = std::move(parts.second);
        } // Anche la distruzione dei risultati temporanei avviene fuori dal tempo.
        state.ResumeTiming();
    }
    state.SetComplexityN(n);
    state.SetItemsProcessed(state.iterations());
}

void BM_Split(benchmark::State& state, std::uint64_t seed) {
    int n = static_cast<int>(state.range(0));
    SkipList list(seed);
    fill(list, n);
    const std::int64_t pivot = n - 1;
    for (auto _ : state) {
        {
            auto parts = list.split(pivot);
            benchmark::DoNotOptimize(parts);
            benchmark::ClobberMemory();
            state.PauseTiming();
            list = SkipList::ordered_union(parts.first, parts.second);
        }
        state.ResumeTiming();
    }
    state.SetComplexityN(n);
    state.SetItemsProcessed(state.iterations());
}

void BM_SplitRebuild(benchmark::State& state, std::uint64_t seed) {
    int n = static_cast<int>(state.range(0));
    // Una ricostruzione completa prima di ogni prova. Solo tre iterazioni:
    // evitiamo una calibrazione illimitata con input grandi.
    for (auto _ : state) {
        state.PauseTiming();
        {
            SkipList list(seed);
            fill(list, n);
            const std::int64_t pivot = n - 1;
            state.ResumeTiming();
            auto parts = list.split_rebuild(pivot);
            benchmark::DoNotOptimize(parts);
            benchmark::ClobberMemory();
            state.PauseTiming();
        } // I vecchi nodi sono gia' stati liberati dal kernel, quelli nuovi qui.
        state.ResumeTiming();
    }
    state.SetComplexityN(n);
    state.SetItemsProcessed(state.iterations());
}

int main(int argc, char** argv) {
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    benchmark::AddCustomContext("compiler", __VERSION__);
    benchmark::AddCustomContext("compile_flags", "see build commands in README.md");
    benchmark::AddCustomContext("height_seeds", "11,42,2026; B uses seed+1000003");
    benchmark::AddCustomContext("workload_seed", "20261023 + N");
    benchmark::AddCustomContext("batches", "search=256, insert=64, others=1");

    for (auto seed : seeds) {
        std::string suffix = "/seed_" + std::to_string(seed);
        benchmark::RegisterBenchmark(("search" + suffix).c_str(), BM_Search, seed)
            ->RangeMultiplier(4)->Range(1024, 1048576)->Iterations(256)->Repetitions(3)->Complexity();
        benchmark::RegisterBenchmark(("insert" + suffix).c_str(), BM_Insert, seed)
            ->RangeMultiplier(4)->Range(1024, 1048576)->Iterations(256)->Repetitions(3)->Complexity();
        benchmark::RegisterBenchmark(("ordered_union" + suffix).c_str(), BM_Union, seed)
            ->RangeMultiplier(4)->Range(1024, 1048576)->Iterations(1000)->Repetitions(3)->Complexity();
        benchmark::RegisterBenchmark(("split" + suffix).c_str(), BM_Split, seed)
            ->RangeMultiplier(4)->Range(1024, 1048576)->Iterations(1000)->Repetitions(3)->Complexity();
        benchmark::RegisterBenchmark(("split_rebuild" + suffix).c_str(), BM_SplitRebuild, seed)
            ->RangeMultiplier(4)->Range(1024, 1048576)->Iterations(3)->Repetitions(3)->Complexity();
    }
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
}
