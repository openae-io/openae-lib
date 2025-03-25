#include <complex>
#include <format>
#include <string>
#include <string_view>
#include <utility>

#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/string.h>
#include <openae/common.hpp>
#include <openae/features.hpp>

#include "common.hpp"

namespace nb = nanobind;

using PyTimedata = nb::ndarray<float, nb::shape<-1>, nb::c_contig>;
using PySpectrum = nb::ndarray<std::complex<float>, nb::shape<-1>, nb::c_contig>;

struct PyInput {
    float samplerate;
    PyTimedata timedata;
    PySpectrum spectrum;

    std::string repr() const {
        return std::format("Input(samplerate={}, timedata=..., spectrum=...)", samplerate);
    }

    std::string str() const {
        return repr();
    }

    operator openae::features::Input() const noexcept {  // NOLINT(*explicit-conversions)
        return {
            .samplerate = samplerate,
            .timedata = {timedata.data(), timedata.size()},
            .spectrum = {spectrum.data(), spectrum.size()},
            .fingerprint = {},
        };
    }

    // special type slots required to fix reference leaks
    // https://nanobind.readthedocs.io/en/latest/refleaks.html
    // https://github.com/wjakob/nanobind/issues/930
    static int tp_traverse(PyObject* self, visitproc visit, void* arg) {
        auto* input = nb::inst_ptr<PyInput>(self);
        Py_VISIT(nb::handle{nb::find(input->timedata)}.ptr());
        Py_VISIT(nb::handle{nb::find(input->spectrum)}.ptr());
#if PY_VERSION_HEX >= 0x03090000
        Py_VISIT(Py_TYPE(self));
#endif
        return 0;
    }

    static int tp_clear(PyObject* self) {
        auto* input = nb::inst_ptr<PyInput>(self);
        input->timedata = {};
        input->spectrum = {};
        return 0;
    }

    // NOLINTNEXTLINE(*c-arrays)
    inline static PyType_Slot slots[] = {
        {Py_tp_traverse, reinterpret_cast<void*>(tp_traverse)},  // NOLINT(*reinterpret-cast)
        {Py_tp_clear, reinterpret_cast<void*>(tp_clear)},  // NOLINT(*reinterpret-cast)
        {0, nullptr},
    };
};

template <typename R, typename... Args>
auto wrap_feature(R (*func)(openae::Env&, openae::features::Input, Args...)) {
    return [func](const PyInput& input, Args&&... args) {
        return func(py_env(), input, std::forward<Args>(args)...);
    };
}

std::string docstring_feature(std::string_view feature_id) {
    return std::format(
        R"(
        Compute feature `{0}`.

        Definition: https://openae.io/standards/features/latest/{0}
        )",
        feature_id
    );
}

NB_MODULE(features, m) {
    m.doc() = "OpenAE feature extraction algorithms.";

    nb::class_<PyInput>(m, "Input", nb::type_slots(static_cast<PyType_Slot*>(PyInput::slots)))
        .def(
            nb::init<float, const PyTimedata&, const PySpectrum&>(),
            nb::arg("samplerate"),
            nb::arg("timedata"),
            nb::arg("spectrum")
        )
        .def_rw("samplerate", &PyInput::samplerate, "Sampling rate in Hz")
        .def_rw("timedata", &PyInput::timedata, "Time-domain signal (typically in volts)")
        .def_rw("spectrum", &PyInput::spectrum, "One-sided spectrum of `timedata`")
        .def("__repr__", &PyInput::repr)
        .def("__str__", &PyInput::str);

    m.def(
        "clearance_factor",
        wrap_feature(openae::features::clearance_factor),
        docstring_feature("clearance-factor").c_str(),
        nb::arg("input")
    );
    m.def(
        "crest_factor",
        wrap_feature(openae::features::crest_factor),
        docstring_feature("crest-factor").c_str(),
        nb::arg("input")
    );
    m.def(
        "energy",
        wrap_feature(openae::features::energy),
        docstring_feature("energy").c_str(),
        nb::arg("input")
    );
    m.def(
        "impulse_factor",
        wrap_feature(openae::features::impulse_factor),
        docstring_feature("impulse-factor").c_str(),
        nb::arg("input")
    );
    m.def(
        "kurtosis",
        wrap_feature(openae::features::kurtosis),
        docstring_feature("kurtosis").c_str(),
        nb::arg("input")
    );
    m.def(
        "partial_power",
        wrap_feature(openae::features::partial_power),
        docstring_feature("partial-power").c_str(),
        nb::arg("input"),
        nb::arg("fmin"),
        nb::arg("fmax")
    );
    m.def(
        "peak_amplitude",
        wrap_feature(openae::features::peak_amplitude),
        docstring_feature("peak-amplitude").c_str(),
        nb::arg("input")
    );
    m.def(
        "rms",
        wrap_feature(openae::features::rms),
        docstring_feature("rms").c_str(),
        nb::arg("input")
    );
    m.def(
        "shape_factor",
        wrap_feature(openae::features::shape_factor),
        docstring_feature("shape-factor").c_str(),
        nb::arg("input")
    );
    m.def(
        "skewness",
        wrap_feature(openae::features::skewness),
        docstring_feature("skewness").c_str(),
        nb::arg("input")
    );
    m.def(
        "spectral_centroid",
        wrap_feature(openae::features::spectral_centroid),
        docstring_feature("spectral-centroid").c_str(),
        nb::arg("input")
    );
    m.def(
        "spectral_entropy",
        wrap_feature(openae::features::spectral_entropy),
        docstring_feature("spectral-entropy").c_str(),
        nb::arg("input")
    );
    m.def(
        "spectral_flatness",
        wrap_feature(openae::features::spectral_flatness),
        docstring_feature("spectral-flatness").c_str(),
        nb::arg("input")
    );
    m.def(
        "spectral_kurtosis",
        wrap_feature(openae::features::spectral_kurtosis),
        docstring_feature("spectral-kurtosis").c_str(),
        nb::arg("input")
    );
    m.def(
        "spectral_peak_frequency",
        wrap_feature(openae::features::spectral_peak_frequency),
        docstring_feature("spectral-peak-frequency").c_str(),
        nb::arg("input")
    );
    m.def(
        "spectral_rolloff",
        wrap_feature(openae::features::spectral_rolloff),
        docstring_feature("spectral-rolloff").c_str(),
        nb::arg("input"),
        nb::arg("rolloff")
    );
    m.def(
        "spectral_skewness",
        wrap_feature(openae::features::spectral_skewness),
        docstring_feature("spectral-skewness").c_str(),
        nb::arg("input")
    );
    m.def(
        "spectral_variance",
        wrap_feature(openae::features::spectral_variance),
        docstring_feature("spectral-variance").c_str(),
        nb::arg("input")
    );
    m.def(
        "zero_crossing_rate",
        wrap_feature(openae::features::zero_crossing_rate),
        docstring_feature("zero-crossing-rate").c_str(),
        nb::arg("input")
    );
}
