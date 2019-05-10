# 2019-May
# github.com/ludlows
# Python Wrapper for PESQ Score (narrow band and wide band)
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


