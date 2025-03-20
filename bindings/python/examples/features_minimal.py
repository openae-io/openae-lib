import numpy as np
import openae.features

# generate random data
rng = np.random.default_rng()
timedata = rng.normal(0, 1, 1024)

# compute spectrum
spectrum = np.fft.rfft(timedata)

# generate input for feature extraction algorithms
input_ = openae.features.Input(
    samplerate=1.0,
    timedata=timedata,
    spectrum=spectrum,
)

# compute features
print(openae.features.rms(input_))
print(openae.features.spectral_centroid(input_))
print(openae.features.spectral_flatness(input_))
