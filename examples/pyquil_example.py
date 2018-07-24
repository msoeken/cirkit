from pyquil.api import CompilerConnection, get_devices
from pyquil.quil import Program
from pyquil.quilbase import Gate
from pyquil.gates import CCNOT

import revkit

# Create a program with RevKit
p = Program()
revkit.perm(permutation="0 2 3 5 7 1 4 6")
revkit.tbs(strategy=2)
p.inst(revkit.to_quil())

# Gates in original program
print("Gates before compilation: {}".format(len(p.instructions)))

# Get a device
agave = get_devices(as_dict=True)['8Q-Agave']
compiler = CompilerConnection(agave)

# Compile
job_id = compiler.compile_async(p)
job = compiler.wait_for_job(job_id, status_time=False)

# Gates in compiled program
qp = job.compiled_quil()

print("Gates after compilation: {}".format(job.gate_volume()))
print("Gate depth after compilation: {}".format(job.gate_depth()))
