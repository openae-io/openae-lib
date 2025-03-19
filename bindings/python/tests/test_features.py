from __future__ import annotations

import sys
from dataclasses import dataclass
from pathlib import Path

import numpy as np
import openae.features
import pytest

if sys.version_info >= (3, 11):
    from tomllib import loads as toml_loads
else:
    from tomli import loads as toml_loads

HERE = Path(__file__).parent
PROJECT_DIR = HERE.parent.parent.parent
TESTS_DIR = PROJECT_DIR / "tests"


@dataclass
class _TestCase:
    name: str
    input_: openae.features.Input
    params: dict[str, any]
    result: float


@dataclass
class _TestConfig:
    feature: str
    tests: list[_TestCase]


def parse_test_config(filename: str):
    filepath = TESTS_DIR / filename
    obj = toml_loads(filepath.read_text())
    return _TestConfig(
        feature=obj["feature"],
        tests=[
            _TestCase(
                name=test["name"],
                input_=openae.features.Input(
                    samplerate=test["input"].get("samplerate", 0.0),
                    timedata=np.asarray(test["input"].get("timedata", [])),
                    spectrum=np.asarray(test["input"].get("spectrum", [])),
                ),
                params=test.get("params", {}),
                result=test["result"],
            )
            for test in obj["tests"]
        ],
    )


def gen_parameter_sets(func, filename: str):
    test_config = parse_test_config(filename)
    for test_case in test_config.tests:
        yield pytest.param(func, test_case, id=f"{test_config.feature}/{test_case.name}")


@pytest.mark.parametrize(
    ("func", "test_case"),
    [
        *gen_parameter_sets(
            openae.features.clearance_factor,
            "test_features_clearance-factor.toml",
        ),
        *gen_parameter_sets(
            openae.features.crest_factor,
            "test_features_crest-factor.toml",
        ),
        *gen_parameter_sets(
            openae.features.energy,
            "test_features_energy.toml",
        ),
        *gen_parameter_sets(
            openae.features.impulse_factor,
            "test_features_impulse-factor.toml",
        ),
        *gen_parameter_sets(
            openae.features.kurtosis,
            "test_features_kurtosis.toml",
        ),
        *gen_parameter_sets(
            openae.features.partial_power,
            "test_features_partial-power.toml",
        ),
        *gen_parameter_sets(
            openae.features.peak_amplitude,
            "test_features_peak-amplitude.toml",
        ),
        *gen_parameter_sets(
            openae.features.rms,
            "test_features_rms.toml",
        ),
        *gen_parameter_sets(
            openae.features.shape_factor,
            "test_features_shape-factor.toml",
        ),
        *gen_parameter_sets(
            openae.features.skewness,
            "test_features_skewness.toml",
        ),
        *gen_parameter_sets(
            openae.features.spectral_centroid,
            "test_features_spectral-centroid.toml",
        ),
        *gen_parameter_sets(
            openae.features.spectral_entropy,
            "test_features_spectral-entropy.toml",
        ),
        *gen_parameter_sets(
            openae.features.spectral_flatness,
            "test_features_spectral-flatness.toml",
        ),
        *gen_parameter_sets(
            openae.features.spectral_kurtosis,
            "test_features_spectral-kurtosis.toml",
        ),
        *gen_parameter_sets(
            openae.features.spectral_peak_frequency,
            "test_features_spectral-peak-frequency.toml",
        ),
        *gen_parameter_sets(
            openae.features.spectral_rolloff,
            "test_features_spectral-rolloff.toml",
        ),
        *gen_parameter_sets(
            openae.features.spectral_skewness,
            "test_features_spectral-skewness.toml",
        ),
        *gen_parameter_sets(
            openae.features.spectral_variance,
            "test_features_spectral-variance.toml",
        ),
        *gen_parameter_sets(
            openae.features.zero_crossing_rate,
            "test_features_zero-crossing-rate.toml",
        ),
    ],
)
def test_features(func, test_case: _TestCase):
    result = func(test_case.input_, **test_case.params)
    if np.isnan(result):
        assert np.isnan(result) == np.isnan(test_case.result)
    else:
        assert result == pytest.approx(test_case.result, rel=1e-6)
