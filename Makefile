venv: venv/bin/activate
venv/bin/activate: requirements.txt
	test -d venv || virtualenv venv
	venv/bin/pip install -r requirements.txt
	touch venv/bin/activate

devbuild: venv
	venv/bin/python setup.py build -b pybuild install

clean:
	-test -d pybuild && rm -Rf pybuild
	-test -d dist && rm -Rf dist
	-test -d cirkit.egg-info && rm -Rf cirkit.egg-info
	-test -d venv && rm -Rf venv
