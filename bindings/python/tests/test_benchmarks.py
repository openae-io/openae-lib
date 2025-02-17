import numpy as np
import openae
import pytest

SAMPLES = 65536


def random_timedata():
    rng = np.random.default_rng(0)
    return rng.uniform(-1, 1, SAMPLES)


def random_input():
    y = random_timedata()
    return openae.features.Input(samplerate=1.0, timedata=y, spectrum=np.fft.rfft(y))


@pytest.mark.benchmark(group="rms")
def test_rms_numpy(benchmark):
    def impl(y):
        return np.sqrt(np.mean(y**2))

    y = random_timedata()
    benchmark(lambda: impl(y))


@pytest.mark.benchmark(group="rms")
def test_rms_openae(benchmark):
    input_ = random_input()
    benchmark(lambda: openae.features.rms(input_))


@pytest.mark.benchmark(group="kurtosis")
def test_kurtosis_numpy(benchmark):
    def impl(y):
        return np.mean((y - y.mean()) ** 4) / (np.var(y) ** 2)

    y = random_timedata()
    benchmark(lambda: impl(y))


@pytest.mark.benchmark(group="kurtosis")
def test_kurtosis_openae(benchmark):
    input_ = random_input()
    benchmark(lambda: openae.features.kurtosis(input_))


@pytest.mark.benchmark(group="spectral_rolloff")
def test_spectral_rolloff_numpy(benchmark):
    def impl(spectrum: np.ndarray, samplerate: float, rolloff: int):
        magnitudes = np.abs(spectrum)
        n = len(magnitudes)
        magnitudes_cumsum = np.cumsum(magnitudes)
        magnitudes_rolloff = rolloff / 100 * magnitudes_cumsum[-1]
        index = np.searchsorted(magnitudes_cumsum, magnitudes_rolloff)
        return 0.5 * samplerate * index / (n - 1)

    input_ = random_input()
    benchmark(lambda: impl(input_.spectrum, input_.samplerate, 0.5))


@pytest.mark.benchmark(group="spectral_rolloff")
def test_spectral_rolloff_openae(benchmark):
    input_ = random_input()
    benchmark(lambda: openae.features.spectral_rolloff(input_, 0.5))
