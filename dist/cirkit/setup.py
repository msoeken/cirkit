from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

__version__ = '3.0a2.dev3'

class get_pybind_include(object):
  """Helper class to determine the pybind11 include path
  The purpose of this class is to postpone importing pybind11
  until it is actually installed, so that the ``get_include()``
  method can be invoked. """

  def __init__(self, user=False):
    self.user = user

  def __str__(self):
    import pybind11
    return pybind11.get_include(self.user)

cirkit_modules = [
  Extension(
    'cirkit',
    ['cli/cirkit.cpp',
     'lib/mockturtle/lib/abcsat/satSolver.cpp',
     'lib/mockturtle/lib/abcsat/satStore.cpp'],
    include_dirs=[
      get_pybind_include(),
      get_pybind_include(user=True),
      "cli",
      "lib/alice/",
      "lib/any",
      "lib/cli11",
      "lib/fmt",
      "lib/json",
      "lib/mockturtle/lib/abcsat",
      "lib/mockturtle/lib/ez",
      "lib/mockturtle/lib/lorina",
      "lib/mockturtle/lib/kitty",
      "lib/mockturtle/lib/percy",
      "lib/mockturtle/lib/rang",
      "lib/mockturtle/lib/sparsepp",
      "lib/mockturtle/include"
    ],
    define_macros=[
      ('ALICE_PYTHON', '1'),
      ('FMT_HEADER_ONLY', '1'),
      ('DISABLE_NAUTY', '1'),
      ('LIN64', '1'),
      ('ABC_NAMESPACE', 'pabc'),
      ('ABC_NO_USE_READLINE', '1')
    ],
    language='c++'
  )
]

class BuildExt(build_ext):
  """A custom build extension for adding compiler-specific options."""
  def build_extensions(self):
    ct = self.compiler.compiler_type
    opts = []
    if ct == 'unix':
      opts.append('-O2')
      opts.append('-DNDEBUG')
      opts.append('-std=c++17')
      opts.append('-Wno-register')
      opts.append('-Wno-unknown-pragmas')
      opts.append('-Wno-unused-variable')
      opts.append('-Wno-deprecated-declarations')
      opts.append('-Wno-format')
      opts.append('-Wno-switch')
    else:
      opts.append('/std:c++17')
    for ext in self.extensions:
      ext.extra_compile_args = opts
    build_ext.build_extensions(self)

with open("README.md", "r") as fh:
  long_description = fh.read()

setup(
  name='cirkit',
  version=__version__,
  author='Mathias Soeken',
  author_email='mathias.soeken@epfl.ch',
  url='https://msoeken.github.io/cirkit.html',
  description='A C++ logic synthesis framework',
  long_description=long_description,
  ext_modules=cirkit_modules,
  install_requires=['pybind11>=2.2'],
  cmdclass={'build_ext': BuildExt},
  zip_safe=False,
  classifiers=(
    "License :: OSI Approved :: MIT License",
    "Operating System :: OS Independent"
  )
)
