from typing import Annotated

from numpy.typing import ArrayLike


class Input:
    def __init__(self, samplerate: float, timedata: Annotated[ArrayLike, dict(dtype='float32', shape=(None), order='C')], spectrum: Annotated[ArrayLike, dict(dtype='complex64', shape=(None), order='C')]) -> None: ...

    @property
    def samplerate(self) -> float:
        """Sampling rate in Hz"""

    @samplerate.setter
    def samplerate(self, arg: float, /) -> None: ...

    @property
    def timedata(self) -> Annotated[ArrayLike, dict(dtype='float32', shape=(None), order='C')]:
        """Time-domain signal (typically in volts)"""

    @timedata.setter
    def timedata(self, arg: Annotated[ArrayLike, dict(dtype='float32', shape=(None), order='C')], /) -> None: ...

    @property
    def spectrum(self) -> Annotated[ArrayLike, dict(dtype='complex64', shape=(None), order='C')]:
        """One-sided spectrum of `timedata`"""

    @spectrum.setter
    def spectrum(self, arg: Annotated[ArrayLike, dict(dtype='complex64', shape=(None), order='C')], /) -> None: ...

    def __repr__(self) -> str: ...

    def __str__(self) -> str: ...

def clearance_factor(input: Input) -> float:
    """
    Compute feature `clearance-factor`.

    Definition: https://openae.io/standards/features/latest/clearance-factor
    """

def crest_factor(input: Input) -> float:
    """
    Compute feature `crest-factor`.

    Definition: https://openae.io/standards/features/latest/crest-factor
    """

def energy(input: Input) -> float:
    """
    Compute feature `energy`.

    Definition: https://openae.io/standards/features/latest/energy
    """

def impulse_factor(input: Input) -> float:
    """
    Compute feature `impulse-factor`.

    Definition: https://openae.io/standards/features/latest/impulse-factor
    """

def kurtosis(input: Input) -> float:
    """
    Compute feature `kurtosis`.

    Definition: https://openae.io/standards/features/latest/kurtosis
    """

def partial_power(input: Input, fmin: float, fmax: float) -> float:
    """
    Compute feature `partial-power`.

    Definition: https://openae.io/standards/features/latest/partial-power
    """

def peak_amplitude(input: Input) -> float:
    """
    Compute feature `peak-amplitude`.

    Definition: https://openae.io/standards/features/latest/peak-amplitude
    """

def rms(input: Input) -> float:
    """
    Compute feature `rms`.

    Definition: https://openae.io/standards/features/latest/rms
    """

def shape_factor(input: Input) -> float:
    """
    Compute feature `shape-factor`.

    Definition: https://openae.io/standards/features/latest/shape-factor
    """

def skewness(input: Input) -> float:
    """
    Compute feature `skewness`.

    Definition: https://openae.io/standards/features/latest/skewness
    """

def spectral_centroid(input: Input) -> float:
    """
    Compute feature `spectral-centroid`.

    Definition: https://openae.io/standards/features/latest/spectral-centroid
    """

def spectral_entropy(input: Input) -> float:
    """
    Compute feature `spectral-entropy`.

    Definition: https://openae.io/standards/features/latest/spectral-entropy
    """

def spectral_flatness(input: Input) -> float:
    """
    Compute feature `spectral-flatness`.

    Definition: https://openae.io/standards/features/latest/spectral-flatness
    """

def spectral_kurtosis(input: Input) -> float:
    """
    Compute feature `spectral-kurtosis`.

    Definition: https://openae.io/standards/features/latest/spectral-kurtosis
    """

def spectral_peak_frequency(input: Input) -> float:
    """
    Compute feature `spectral-peak-frequency`.

    Definition: https://openae.io/standards/features/latest/spectral-peak-frequency
    """

def spectral_rolloff(input: Input, rolloff: float) -> float:
    """
    Compute feature `spectral-rolloff`.

    Definition: https://openae.io/standards/features/latest/spectral-rolloff
    """

def spectral_skewness(input: Input) -> float:
    """
    Compute feature `spectral-skewness`.

    Definition: https://openae.io/standards/features/latest/spectral-skewness
    """

def spectral_variance(input: Input) -> float:
    """
    Compute feature `spectral-variance`.

    Definition: https://openae.io/standards/features/latest/spectral-variance
    """

def zero_crossing_rate(input: Input) -> float:
    """
    Compute feature `zero-crossing-rate`.

    Definition: https://openae.io/standards/features/latest/zero-crossing-rate
    """
