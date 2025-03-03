#include <array>
#include <memory_resource>
#include <vector>

#include <benchmark/benchmark.h>

#include "openae/common.hpp"
#include "openae/features.hpp"

#include "random.hpp"

static constexpr size_t buffer_size = 10'000'000;

class AllocationCounter : public std::pmr::memory_resource {
public:
    explicit AllocationCounter(std::pmr::memory_resource* upstream) noexcept
        : upstream_(upstream) {}

    size_t allocated_bytes() const noexcept {
        return allocated_bytes_;
    }

protected:
    void* do_allocate(size_t bytes, size_t alignment) {
        allocated_bytes_ += bytes;
        return upstream_->allocate(bytes, alignment);
    }

    void do_deallocate(void* p, size_t bytes, size_t alignment) {
        upstream_->deallocate(p, bytes, alignment);
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept {
        return upstream_->is_equal(other);
    }

private:
    std::pmr::memory_resource* upstream_;
    size_t allocated_bytes_{0};
};

struct OwningInput {
    float samplerate;
    std::vector<float> timedata;
    std::vector<std::complex<float>> spectrum;

    operator openae::features::Input() const {
        return {
            .samplerate = samplerate,
            .timedata = timedata,
            .spectrum = spectrum,
            .fingerprint = {},
        };
    }
};

static OwningInput make_random_input(float samplerate, size_t size) {
    return OwningInput{
        .samplerate = samplerate,
        .timedata = make_random_vector<float>(size, -1.0, 1.0),
        .spectrum = make_random_vector<std::complex<float>>(size, -1.0, 1.0),
    };
}

template <typename Func, typename... Args>
static void run_default(benchmark::State& state, Func&& func, Args... args) {
    AllocationCounter new_delete_resource{std::pmr::new_delete_resource()};
    openae::Env env{};
    env.mem_resource = &new_delete_resource;

    const auto input = make_random_input(1, state.range(0));
    for (auto _ : state) {
        auto result = func(env, input, args...);
        benchmark::DoNotOptimize(result);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.counters["allocated_bytes"] = static_cast<double>(new_delete_resource.allocated_bytes());
}

template <typename Func, typename... Args>
static void run_cached(benchmark::State& state, Func&& func, Args... args) {
    AllocationCounter new_delete_resource{std::pmr::new_delete_resource()};
    auto cache = openae::make_cache();
    openae::Env env{};
    env.mem_resource = &new_delete_resource;
    env.cache = cache.get();

    const auto input = make_random_input(1, state.range(0));
    for (auto _ : state) {
        auto result = func(env, input, args...);
        benchmark::DoNotOptimize(result);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.counters["allocated_bytes"] = static_cast<double>(new_delete_resource.allocated_bytes());
}

template <typename Func, typename... Args>
static void run_monotonic(benchmark::State& state, Func&& func, Args... args) {
    std::vector<std::byte> buffer(buffer_size);
    AllocationCounter new_delete_resource{std::pmr::new_delete_resource()};

    const auto input = make_random_input(1, state.range(0));
    for (auto _ : state) {
        state.PauseTiming();
        std::pmr::monotonic_buffer_resource buffer_resource{buffer.data(), buffer.size(), &new_delete_resource};
        openae::Env env{};
        env.mem_resource = &buffer_resource;
        state.ResumeTiming();

        auto result = func(env, input, args...);
        benchmark::DoNotOptimize(result);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.counters["allocated_bytes"] = static_cast<double>(new_delete_resource.allocated_bytes());
}

template <typename Func, typename... Args>
static void run_pool(benchmark::State& state, Func&& func, Args... args) {
    std::vector<std::byte> buffer(buffer_size);
    AllocationCounter new_delete_resource{std::pmr::new_delete_resource()};
    std::pmr::monotonic_buffer_resource buffer_resource{buffer.data(), buffer.size(), &new_delete_resource};
    std::pmr::unsynchronized_pool_resource pool_resource{&buffer_resource};
    openae::Env env{};
    env.mem_resource = &pool_resource;

    const auto input = make_random_input(1, state.range(0));
    for (auto _ : state) {
        auto result = func(env, input, args...);
        benchmark::DoNotOptimize(result);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.counters["allocated_bytes"] = static_cast<double>(new_delete_resource.allocated_bytes());
}

constexpr size_t vec_size = 65536;

BENCHMARK_CAPTURE(run_default, peak_amplitude, openae::features::peak_amplitude)->Arg(vec_size);
BENCHMARK_CAPTURE(run_default, energy, openae::features::energy)->Arg(vec_size);
BENCHMARK_CAPTURE(run_default, rms, openae::features::rms)->Arg(vec_size);
BENCHMARK_CAPTURE(run_default, crest_factor, openae::features::crest_factor)->Arg(vec_size);
BENCHMARK_CAPTURE(run_default, impulse_factor, openae::features::impulse_factor)->Arg(vec_size);
BENCHMARK_CAPTURE(run_default, clearance_factor, openae::features::clearance_factor)->Arg(vec_size);
BENCHMARK_CAPTURE(run_default, shape_factor, openae::features::shape_factor)->Arg(vec_size);
BENCHMARK_CAPTURE(run_default, skewness, openae::features::skewness)->Arg(vec_size);
BENCHMARK_CAPTURE(run_default, kurtosis, openae::features::kurtosis)->Arg(vec_size);
BENCHMARK_CAPTURE(run_default, zero_crossing_rate, openae::features::zero_crossing_rate)->Arg(vec_size);
BENCHMARK_CAPTURE(run_default, partial_power, openae::features::partial_power, 0.1f, 0.2f)->Arg(vec_size);
BENCHMARK_CAPTURE(run_default, spectral_peak_frequency, openae::features::spectral_peak_frequency)->Arg(vec_size);
BENCHMARK_CAPTURE(run_default, spectral_centroid, openae::features::spectral_centroid)->Arg(vec_size);
BENCHMARK_CAPTURE(run_default, spectral_variance, openae::features::spectral_variance)->Arg(vec_size);
BENCHMARK_CAPTURE(run_default, spectral_skewness, openae::features::spectral_skewness)->Arg(vec_size);
BENCHMARK_CAPTURE(run_default, spectral_kurtosis, openae::features::spectral_kurtosis)->Arg(vec_size);
BENCHMARK_CAPTURE(run_default, spectral_rolloff, openae::features::spectral_rolloff, 0.9f)->Arg(vec_size);
BENCHMARK_CAPTURE(run_default, spectral_flatness, openae::features::spectral_flatness)->Arg(vec_size);

BENCHMARK_CAPTURE(run_monotonic, spectral_rolloff, openae::features::spectral_rolloff, 0.9f)->Arg(vec_size);
BENCHMARK_CAPTURE(run_pool, spectral_rolloff, openae::features::spectral_rolloff, 0.9f)->Arg(vec_size);

BENCHMARK_MAIN();
