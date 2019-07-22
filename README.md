# python-pesq

[![Build Status](https://travis-ci.org/boeddeker/python-pesq.svg?branch=master)](https://travis-ci.org/boeddeker/python-pesq)

PESQ (Perceptual Evaluation of Speech Quality) Wrapper for Python Users

This code is designed for numpy array specially.

# Requirements

    gcc compiler
    numpy
    cython

# Build and Install
```bash
$ git clone https://github.com/ludlows/python-pesq.git
$ cd python-pesq
$ pip install .  # for python 2
$ pip3 install . # for python 3 
```

# Install with pip
```
$ pip install https://github.com/ludlows/python-pesq/archive/master.zip

# or

$ pip3 install https://github.com/ludlows/python-pesq/archive/master.zip

```

# Example for narrow band and wide band

```python
from scipy.io import wavfile
from pypesq import pypesq

rate, ref = wavfile.read("./audio/speech.wav")
rate, deg = wavfile.read("./audio/speech_bab_0dB.wav")

print(pypesq(rate, ref, deg, 'wb'))
print(pypesq(rate, ref, deg, 'nb'))
```

# Correctness

The correctness is verified by running samples in audio folder.

PESQ computed by this code in wide band mode is    1.0832337141036987

PESQ computed by this code in narrow band mode is  1.6072081327438354

# Note

Sampling rate (fs|rate) - No default. Must select either 8000Hz or 16000Hz.
 
Note there is narrow band (nb) mode only when sampling rate is 8000Hz.

The original C soure code is modified. 
