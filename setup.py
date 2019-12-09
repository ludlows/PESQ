# 2019-May
# github.com/ludlows
# Python Wrapper for PESQ Score (narrow band and wide band)
import numpy

from setuptools import find_packages
from distutils.core import setup
from distutils.extension import Extension

from Cython.Build import cythonize, build_ext

with open("README.md", "r") as fh:
    long_description = fh.read()

extensions = [
    Extension(
        "cypesq",
        ["pesq/cypesq.pyx", "pesq/dsp.c", "pesq/pesqdsp.c","pesq/pesqmod.c"],
        include_dirs=['pesq', numpy.get_include()],
        language="c")
]
setup(
    name="pesq",
    version="0.0.1",
    author="ludlows",
    description="Python Wrapper for PESQ Score (narrow band and wide band)",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/ludlows/python-pesq",
    packages=find_packages(),
    package_data={'pesq':["*.pyx", "*.h", "dsp.c", "pesqdsp.c", "pesqmod.c"]},
    # cmdclass = {'build_ext': build_ext},
    ext_package='pesq',
    ext_modules=cythonize(extensions),
    setup_requires=['numpy', 'cython', 'pytest-runner'],
    tests_require=['pytest'],
    classifiers=[
        "Programming Language :: Python",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ]
)


