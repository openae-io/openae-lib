#include <complex>
#include <memory_resource>
#include <string>
#include <utility>

#include <fmt/format.h>
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/string.h>

#include <openae/common.hpp>
#include <openae/features.hpp>

namespace nb = nanobind;

using PyTimedata = nb::ndarray<float, nb::shape<-1>, nb::c_contig>;
using PySpectrum = nb::ndarray<std::complex<float>, nb::shape<-1>, nb::c_contig>;

struct PyInput {
    float samplerate;
    PyTimedata timedata;
    PySpectrum spectrum;

    std::string repr() const {
        return fmt::format("Input(samplerate={}, timedata=..., spectrum=...)", samplerate);
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

constexpr int py_log_level(openae::LogLevel level) noexcept {
    // https://docs.python.org/3/library/logging.html#logging-levels
    switch (level) {
    case openae::LogLevel::Trace:
    case openae::LogLevel::Debug:
        return 10;
    case openae::LogLevel::Info:
        return 20;
    case openae::LogLevel::Warning:
        return 30;
    case openae::LogLevel::Error:
        return 40;
    case openae::LogLevel::Fatal:
        return 50;
    default:
        return 0;
    }
}

void py_log(openae::LogLevel level, const char* msg) {
    auto logger = nb::module_::import_("logging").attr("getLogger")("openae");
    // https://docs.python.org/3/library/logging.html#logrecord-objects
    nb::dict kwargs;  // NOLINT(*const-correctness), false positive
    kwargs["extra"] = nb::dict{};
    logger.attr("log")(py_log_level(level), msg, **kwargs);
}

openae::Env& py_env() {
    static openae::Env env{
        .logger = py_log,
        .mem_resource = std::pmr::new_delete_resource(),
        .cache = nullptr,
    };
    return env;
}

template <typename R, typename... Args>
auto wrap_feature(R (*func)(openae::Env&, openae::features::Input, Args...)) {
    return [func](const PyInput& input, Args&&... args) {
        return func(py_env(), input, std::forward<Args>(args)...);
    };
}

NB_MODULE(openae, m) {
    auto m_features = m.def_submodule("features");

    nb::class_<PyInput>(
        m_features, "Input", nb::type_slots(static_cast<PyType_Slot*>(PyInput::slots))
    )
        .def(
            nb::init<float, const PyTimedata&, const PySpectrum&>(),
            nb::arg("samplerate"),
            nb::arg("timedata"),
            nb::arg("spectrum")
        )
        .def_rw("samplerate", &PyInput::samplerate)
        .def_rw("timedata", &PyInput::timedata)
        .def_rw("spectrum", &PyInput::spectrum)
        .def("__repr__", &PyInput::repr)
        .def("__str__", &PyInput::str);

    // clang-format off
    m_features.def("clearance_factor", wrap_feature(openae::features::clearance_factor), nb::arg("input"));
    m_features.def("crest_factor", wrap_feature(openae::features::crest_factor), nb::arg("input"));
    m_features.def("energy", wrap_feature(openae::features::energy), nb::arg("input"));
    m_features.def("impulse_factor", wrap_feature(openae::features::impulse_factor), nb::arg("input"));
    m_features.def("kurtosis", wrap_feature(openae::features::kurtosis), nb::arg("input"));
    m_features.def("partial_power", wrap_feature(openae::features::partial_power), nb::arg("input"), nb::arg("fmin"), nb::arg("fmax"));
    m_features.def("peak_amplitude", wrap_feature(openae::features::peak_amplitude), nb::arg("input"));
    m_features.def("rms", wrap_feature(openae::features::rms), nb::arg("input"));
    m_features.def("shape_factor", wrap_feature(openae::features::shape_factor), nb::arg("input"));
    m_features.def("skewness", wrap_feature(openae::features::skewness), nb::arg("input"));
    m_features.def("spectral_centroid", wrap_feature(openae::features::spectral_centroid), nb::arg("input"));
    m_features.def("spectral_entropy", wrap_feature(openae::features::spectral_entropy), nb::arg("input"));
    m_features.def("spectral_flatness", wrap_feature(openae::features::spectral_flatness), nb::arg("input"));
    m_features.def("spectral_kurtosis", wrap_feature(openae::features::spectral_kurtosis), nb::arg("input"));
    m_features.def("spectral_peak_frequency", wrap_feature(openae::features::spectral_peak_frequency), nb::arg("input"));
    m_features.def("spectral_rolloff", wrap_feature(openae::features::spectral_rolloff), nb::arg("input"), nb::arg("rolloff"));
    m_features.def("spectral_skewness", wrap_feature(openae::features::spectral_skewness), nb::arg("input"));
    m_features.def("spectral_variance", wrap_feature(openae::features::spectral_variance), nb::arg("input"));
    m_features.def("zero_crossing_rate", wrap_feature(openae::features::zero_crossing_rate), nb::arg("input"));
    // clang-format on
}
