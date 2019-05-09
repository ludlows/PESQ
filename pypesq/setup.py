import numpy

from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize, build_ext

extensions = [
    Extension(
        "cypesq",
        ["cypesq.pyx", "dsp.c", "pesqdsp.c","pesqmod.c"],
        include_dirs=[numpy.get_include()],
        language="c")
]
setup(
    name="PESQ Python Wrapper",
    cmdclass = {'build_ext': build_ext},
    ext_modules=cythonize(extensions),
)

#python setup.py build_ext --inplace
# import numpy as np
# from scipy.io import wavfile
# rate, ref = wavfile.read("../audio/speech.wav")
# rate, deg = wavfile.read("../audio/speech_bab_0dB.wav")
# maxval = max(np.max(np.abs(ref/1.0)), np.max(np.abs(deg/1.0)));
# cypesq(rate, (ref/maxval).astype(np.float32),(deg/maxval).astype(np.float32), 0)
# ref = ref.astype(np.float32) / 2**15;
# deg = deg.astype(np.float32) / 2**15

