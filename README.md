# python-pesq

PESQ (Perceptual Evaluation of Speech Quality) Wrapper for Python Users

# Requirements

    gcc compiler
    numpy
    cython

# build and install
```bash
$ git clone https://github.com/ludlows/python-pesq.git
$ cd python-pesq/pypesq
$ python setup.py build_ext --inplace
$ cd ..
```


# example for narrow band and wide band

```python
from scipy.io import wavfile
from pypesq import pypesq

rate, ref = wavfile.read("./audio/speech.wav")
rate, deg = wavfile.read("./audio/speech_bab_0dB.wav")

print(pypesq(rate, ref, deg, 'wb'))
print(pypesq(rate, ref, deg, 'nb'))
```

