#!/bin/bash

eval "$(pyenv init -)"

mkdir -p wheels

pyenv shell 2.7.15
python --version
rm -Rf cirkit/build cirkit/cirkit.egg-info cirkit/dist
python -m pip install --upgrade pip
python -m pip install pybind11
python -m pip install wheel

cd cirkit
python setup.py bdist_wheel
cp dist/*.whl ../wheels
cd ..

pyenv shell 3.4.8
python --version
rm -Rf cirkit/build cirkit/cirkit.egg-info cirkit/dist
python -m pip install --upgrade pip
python -m pip install pybind11
python -m pip install wheel

cd cirkit
python setup.py bdist_wheel
cp dist/*.whl ../wheels
cd ..

pyenv shell 3.5.5
python --version
rm -Rf cirkit/build cirkit/cirkit.egg-info cirkit/dist
python -m pip install --upgrade pip
python -m pip install pybind11
python -m pip install wheel

cd cirkit
python setup.py bdist_wheel
cp dist/*.whl ../wheels
cd ..

pyenv shell 3.6.5
python --version
rm -Rf cirkit/build cirkit/cirkit.egg-info cirkit/dist
python -m pip install --upgrade pip
python -m pip install pybind11
python -m pip install wheel

cd cirkit
python setup.py bdist_wheel
cp dist/*.whl ../wheels
cd ..

pyenv shell 3.7.0
python --version
rm -Rf cirkit/build cirkit/cirkit.egg-info cirkit/dist
python -m pip install --upgrade pip
python -m pip install pybind11
python -m pip install wheel

cd cirkit
python setup.py bdist_wheel
cp dist/*.whl ../wheels
cd ..
