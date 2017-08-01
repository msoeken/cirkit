# ext_qpic.py

import os
import subprocess

from IPython.core.magic import Magics, magics_class, cell_magic
from IPython.display import display, Image
from tempfile import NamedTemporaryFile

@magics_class
class qpic_magics(Magics):
    @cell_magic
    def qpic(self, line, cell):
        f = NamedTemporaryFile(delete = False, prefix = "ext_qpic", suffix = '.qpic', dir = '.')
        name = os.path.basename(f.name)
        f.write(cell.encode())
        f.close()

        subprocess.call(['qpic', '-f', 'png', name])
        os.unlink(name)

        display(Image(name[:-4] + "png"))

def load_ipython_extension(ipython):
    ipython.register_magics(qpic_magics)

def unload_ipython_extension(ipython):
    pass
