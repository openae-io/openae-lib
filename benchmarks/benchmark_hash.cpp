#include <cmath>
#include <numeric>
#include <vector>

#include <benchmark/benchmark.h>
#define XXH_INLINE_ALL
#include <xxhash.h>

#include "random.hpp"

static void benchmark_sum(benchmark::State& state) {
    const auto vec = make_random_vector<float>(state.range(), -1.0, 1.0);
    for (auto _ : state) {
        const auto sum = std::accumulate(vec.begin(), vec.end(), 0.0f);
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}

static void benchmark_std_hash(benchmark::State& state) {
    const auto vec = make_random_vector<float>(state.range(), -1.0, 1.0);
    for (auto _ : state) {
        size_t hash = vec.size();
        for (auto value : vec) {
            hash ^= std::hash<float>{}(value);
        }
        benchmark::DoNotOptimize(hash);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}

template <typename F>
static void benchmark_xxhash(benchmark::State& state, F hash_func) {
    const auto vec = make_random_vector<float>(state.range(), -1.0, 1.0);
    for (auto _ : state) {
        const auto hash = hash_func(vec.data(), vec.size() * sizeof(float), 0 /* seed */);
        benchmark::DoNotOptimize(hash);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}

constexpr size_t vec_size = 65536;
BENCHMARK(benchmark_sum)->Arg(vec_size);
BENCHMARK(benchmark_std_hash)->Arg(vec_size);
BENCHMARK_CAPTURE(benchmark_xxhash, xxh32, XXH32)->Arg(vec_size);
BENCHMARK_CAPTURE(benchmark_xxhash, xxh32, XXH64)->Arg(vec_size);
#ifdef XXH128
BENCHMARK_CAPTURE(benchmark_xxhash, xxh32, XXH128)->Arg(vec_size);
#endif

BENCHMARK_MAIN();
