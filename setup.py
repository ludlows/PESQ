# 2019-May
# github.com/ludlows
# Python Wrapper for PESQ Score (narrow band and wide band)
from setuptools import find_packages
from setuptools import setup, Extension


with open("README.md", "r") as fh:
    long_description = fh.read()


class CyPesqExtension(Extension):
    def __init__(self, *args, **kwargs):
        self._include = []
        super().__init__(*args, **kwargs)

    @property
    def include_dirs(self):
        import numpy
        return self._include + [numpy.get_include()]

    @include_dirs.setter
    def include_dirs(self, dirs):
        self._include = dirs


extensions = [
    CyPesqExtension(
        "cypesq",
        ["pesq/cypesq.pyx", "pesq/dsp.c", "pesq/pesqdsp.c","pesq/pesqmod.c"],
        include_dirs=['pesq'],
        language="c")
]
setup(
    name="pesq",
    version="0.0.3",
    author="ludlows",
    description="Python Wrapper for PESQ Score (narrow band and wide band)",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/ludlows/python-pesq",
    packages=find_packages(),
    package_data={'pesq':["*.pyx", "*.h", "dsp.c", "pesqdsp.c", "pesqmod.c"]},
    ext_package='pesq',
    ext_modules=extensions,
    setup_requires=['setuptools>=18.0', 'cython', 'numpy', 'pytest-runner'],
    tests_require=['pytest'],
    classifiers=[
        "Programming Language :: Python",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ]
)
