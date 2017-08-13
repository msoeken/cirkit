Integration
===========

CirKit offers functions to interact with certain tools. This section
shows with which tools CirKit interacts well and gives some
illustrative examples.

ABC
---

CirKit is tightly integrated with ABC. ABC can be accessed as a
subshell inside CirKit with the command ``abc``. If an AIG is present
in CirKit's AIG store, it will be copied to the ABC subshell and
available in the &-space (ABC9 commands). Furthermore, when leaving
the ABC subshell using ``quit`` ABC's AIG will be copied back to
CirKit and replace the current AIG in the store (unless ``abc`` is
called with the option ``-n``). The following example illustrates this
interaction in which an AIG is copied to ABC in order to optimize it
and then copied back:

.. code-block:: cirkit

   cirkit> read_aiger c432.aig
   cirkit> ps -a
   [i]                 c432: i/o =      36 /       7  and =     136  lev =   25
   cirkit> abc
   UC Berkeley, ABC 1.01 (compiled Apr 22 2016 19:45:32)
   abc 01> &ps
   c432     : i/o =     36/      7  and =     136  lev =   25 (19.14)  mem = 0.00 MB
   abc 01> &dc2
   abc 01> &ps
   c432     : i/o =     36/      7  and =     123  lev =   25 (19.14)  mem = 0.00 MB
   abc 01> quit
   cirkit> ps -a
   [i]                 c432: i/o =      36 /       7  and =     123  lev =   25
   cirkit> quit
