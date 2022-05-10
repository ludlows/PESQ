# pesq
[![Downloads](https://pepy.tech/badge/pesq)](https://pepy.tech/project/pesq)
[![Downloads](https://pepy.tech/badge/pesq/month)](https://pepy.tech/project/pesq)

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

# Usage for narrowband and wideband Modes

Please note that the sampling rate (frequency) should be 16000 or 8000 (Hz). 

And using 8000Hz is supported for narrowband only.

The code supports error-handling behaviors now.

```python
def pesq(fs, ref, deg, mode='wb', on_error=PesqError.RAISE_EXCEPTION):
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

# Usage for `multiprocessing` feature

```python
def pesq_batch(fs, ref, deg, mode='wb', n_processor=None, on_error=PesqError.RAISE_EXCEPTION):
    """
   Running `pesq` using multiple processors
    Args:
        on_error:
        ref: numpy 1D (n_sample,) or 2D array (n_file, n_sample), reference audio signal
        deg: numpy 1D (n_sample,) or 2D array (n_file, n_sample), degraded audio signal
        fs:  integer, sampling rate
        mode: 'wb' (wide-band) or 'nb' (narrow-band)
        n_processor: cpu_count() (default) or number of processors (chosen by the user) or 0 (without multiprocessing)
        on_error: PesqError.RAISE_EXCEPTION (default) or PesqError.RETURN_VALUES
    Returns:
        pesq_score: list of pesq scores, P.862.2 Prediction (MOS-LQO)
    """
```
this function uses `multiprocessing` features to boost time efficiency.

When the `ref` is an 1-D numpy array and `deg` is a 2-D numpy array, the result of `pesq_batch` is identical to the value of `[pesq(fs, ref, deg[i,:],**kwargs) for i in range(deg.shape[0])]`.

When the `ref` is a 2-D numpy array and `deg` is a 2-D numpy array, the result of `pesq_batch` is identical to the value of `[pesq(fs, ref[i,:], deg[i,:],**kwargs) for i in range(deg.shape[0])]`.


# Correctness

The correctness is verified by running samples in audio folder.

PESQ computed by this code in wideband mode is    1.0832337141036987

PESQ computed by this code in narrowband mode is  1.6072081327438354

# Note

Sampling rate (fs|rate) - No default. Must select either 8000Hz or 16000Hz.
 
Note there is narrowband (nb) mode only when sampling rate is 8000Hz.

The original C source code is modified. 

# Who is using `pesq`

Please click [here](https://github.com/ludlows/python-pesq/network/dependents) to see these repositories, whose owners include `Facebook Research`, `SpeechBrain`, `NVIDIA` .etc.

# Acknowledgement

This work was funded by the Natural Sciences and Engineering Research Council of Canada.

This work was also funded by the Concordia University, Montreal, Canada.