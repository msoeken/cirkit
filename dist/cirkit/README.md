# CirKit (Python interface)

CirKit is a synthesis and optimization frameworks for classical logic networks.
It is implemented based on various
[EPFL logic sythesis libraries](https://github.com/lsils/lstools-showcase).

## Example

```python
import cirkit
cirkit.read_aiger(aig=True, filename="file.aig")
cirkit.ps(aig=True)
cirkit.cut_rewrite(aig=True)
cirkit.lut_mapping(aig=True)
cirkit.collapse_mapped(aig=True)
cirkit.ps(lut=True)
cirkit.write_bench(lut=True, filename="file.bench")
```

## EPFL logic sythesis libraries

CirKit and Revkit are based on the [EPFL logic synthesis](https://lsi.epfl.ch/page-138455-en.html) libraries.  The libraries and several examples on how to use and integrate the libraries can be found in the [logic synthesis tool showcase](https://github.com/lsils/lstools-showcase).
