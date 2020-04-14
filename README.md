# python-pesq

PESQ (Perceptual Evaluation of Speech Quality) Wrapper for Python.

This code is designed for numpy arrays specifically.

# Requirements

    C compiler
    numpy
    cython

# Install with pip

```bash
# PyPi Repository
$ pip install pesq


# The Latest Version
$ pip install https://github.com/ludlows/python-pesq/archive/master.zip

# or

$ pip3 install https://github.com/ludlows/python-pesq/archive/master.zip
```

# Build and install latest
```bash
$ git clone https://github.com/ludlows/python-pesq.git
$ cd python-pesq
$ pip install .  # for python 2
$ pip3 install . # for python 3 
$ cd ..
$ rm -rf python-pesq # remove the code folder since it exists in the python package folder
```


# Example for narrow band and wide band

Please note that the sampling rate (frequency) should be 16000 or 8000 
and 8000 is supported for narrow band only.

```python
from scipy.io import wavfile
from pesq import pesq

rate, ref = wavfile.read("./audio/speech.wav")
rate, deg = wavfile.read("./audio/speech_bab_0dB.wav")

print(pesq(rate, ref, deg, 'wb'))
print(pesq(rate, ref, deg, 'nb'))
```

# Correctness

The correctness is verified by running samples in audio folder.

PESQ computed by this code in wide band mode is    1.0832337141036987

PESQ computed by this code in narrow band mode is  1.6072081327438354

# Note

Sampling rate (fs|rate) - No default. Must select either 8000Hz or 16000Hz.
 
Note there is narrow band (nb) mode only when sampling rate is 8000Hz.

The original C soure code is modified.

If the reference sound is nearly silent then you will get an
exception `No utterances detected` that you can handle manually if you want.

```python
from pesq import PESQError

SAMPLE_RATE = 16000
silent_ref = np.zeros(16000)
deg = np.random.randn(16000)

try:
    pesq(SAMPLE_RATE, silent_ref, deg, 'wb')
except PESQError as e:
    print(e)
```
