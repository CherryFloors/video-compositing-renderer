"""setup.py"""

import os
from pathlib import Path

from setuptools import setup, Extension


define_macros = [("_REENTRANT", None)]
if os.getenv("DEBUG", 0):
    define_macros.append(("DEBUG", None))

LIB_VCR = Extension(
    name="vcr._libvcr",
    sources=["c/_libvcr.c"],
    include_dirs=["/usr/include/SDL2", str(Path("c").resolve())],
    define_macros=define_macros,
    libraries=["mpv", "SDL2", "SDL2_ttf", "SDL2_image"],
)

setup(ext_modules=[LIB_VCR])
