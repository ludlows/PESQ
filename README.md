# python-pesq

PESQ (Perceptual Evaluation of Speech Quality) Wrapper for Python Users

This code is designed for numpy array specially.

# Requirements

    C compiler
    numpy
    cython

# Build and Install
```bash
$ git clone https://github.com/ludlows/python-pesq.git
$ cd python-pesq
$ pip install .  # for python 2
$ pip3 install . # for python 3 
$ cd ..
$ rm -rf python-pesq # remove the code folder since it exists in the python package folder
```

# Install with pip

```bash
# PyPi Repository
$ pip install pesq


# The Latest Version
$ pip install https://github.com/ludlows/python-pesq/archive/master.zip

# or

$ pip3 install https://github.com/ludlows/python-pesq/archive/master.zip
```

# Usage for narrow-band and wide-band Modes

Please note that the sampling rate (frequency) should be 16000 or 8000 (Hz). 

And using 8000Hz is supported for narrow band only.

The code supports error-handling behaviors now.

```python
def pesq(fs, ref, deg, mode, on_error=PesqError.RAISE_EXCEPTION):
    """
    Args:
        ref: numpy 1D array, reference audio signal 
        deg: numpy 1D array, degraded audio signal
        fs:  integer, sampling rate
        mode: 'wb' (wide-band) or 'nb' (narrow-band)
        on_error: error-handling behavior, it could be PesqError.RETURN_VALUES or PesqError.RAISE_EXCEPTION by default
    Returns:
        pesq_score: float, P.862.2 Prediction (MOS-LQO)
    """
```
Once you select `PesqError.RETURN_VALUES`, the `pesq` function will return -1 when an error occurs.

Once you select `PesqError.RAISE_EXCEPTION`, the `pesq` function will raise an exception when an error occurs.

It supports the following errors now: `InvalidSampleRateError`, `OutOfMemoryError`,`BufferTooShortError`,`NoUtterancesError`,`PesqError`(other unknown errors).

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

# Who is using `python-pesq`
Please click [here](https://github.com/ludlows/python-pesq/network/dependents) to see these repositories, whose owners include `Facebook Research`, `SpeechBrain`, `NVIDIA` .etc.


# Buy me a Coffee
[Buy](https://www.paypal.me/wangmiao521) me a Coffee if my work helps you in some ways.
