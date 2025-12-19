"""setup.py"""

import os
from pathlib import Path

from setuptools import setup, Extension


define_macros = [("_REENTRANT", None)]
if os.getenv("DEBUG", 0):
    define_macros.append(("DEBUG", None))

lib_vcr = Extension(
    name="vcr._libvcr",
    sources=["c/_libvcr.c"],
    include_dirs=["/usr/include/SDL2", str(Path("c").resolve())],
    define_macros=define_macros,
    libraries=["mpv", "SDL2"],
)

setup(ext_modules=[lib_vcr])
