# 2019-May
# github.com/ludlows
# Python Wrapper for PESQ Score (narrow band and wide band)
import numpy

from setuptools import find_packages
from distutils.core import setup
from distutils.extension import Extension

from Cython.Build import cythonize, build_ext

extensions = [
    Extension(
        "cypesq",
        ["pypesq/cypesq.pyx", "pypesq/dsp.c", "pypesq/pesqdsp.c","pypesq/pesqmod.c"],
        include_dirs=[numpy.get_include()],
        language="c")
]
setup(
    name="pypesq",
    packages=find_packages(),
    # cmdclass = {'build_ext': build_ext},
    ext_package='pypesq',
    ext_modules=cythonize(extensions),
    setup_requires=['pytest-runner'],
    tests_require=['pytest', 'soundfile'],
)


