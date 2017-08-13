Data Structures
===============

CirKit (and RevKit) provide the analysis and manipulation of several
data structures. These data structures are explained in this
section. As described above, instances of these data structures are
stored in individual stores. Not all data structures are available in
both CirKit and RevKit. The following table gives an overview over the
existing data structures, their access option for the store, and their
availability in CirKit and RevKit.

+-----------------------------------------------------+-----------------------+---------+---------+
| Data structure                                      | Access option         | CirKit  | RevKit  |
+=====================================================+=======================+=========+=========+
| Truth table                                         | ``-t`` ``--tt``       | |check| | |check| |
+-----------------------------------------------------+-----------------------+---------+---------+
| Expression                                          | ``-e`` ``--expr``     | |check| | |check| |
+-----------------------------------------------------+-----------------------+---------+---------+
| And-inverter graph (AIG)                            | ``-a`` ``--aig``      | |check| | |check| |
+-----------------------------------------------------+-----------------------+---------+---------+
| Majority-inverter graph (MIG)                       | ``-m`` ``--mig``      | |check| |         |
+-----------------------------------------------------+-----------------------+---------+---------+
| XOR majority graph (XMG)                            | ``-x`` ``--xmg``      | |check| | |check| |
+-----------------------------------------------------+-----------------------+---------+---------+
| Binary decision diagram (BDD)                       | ``-b`` ``--bdd``      | |check| | |check| |
+-----------------------------------------------------+-----------------------+---------+---------+
| Reversible circuit                                  | ``-c`` ``--circuit``  |         | |check| |
+-----------------------------------------------------+-----------------------+---------+---------+
| Reversible specification                            | ``-s`` ``--spec``     |         | |check| |
+-----------------------------------------------------+-----------------------+---------+---------+
| BDD of a characteristic reversible function (RCBDD) | ``-r`` ``--rcbdd``    |         | |check| |
+-----------------------------------------------------+-----------------------+---------+---------+

Truth tables
------------

Truth tables are bitstrings of length $2^k$ and represent Boolean
functions over $k$ variables. The most significant bit is the first
bit in the bitstring. For example, to load a truth table that
represents the AND function $a \land b$, type ``tt 1000``. We assume
that the least significant variable is $a$, then $b$, then $c$, and so
on. The truth tables for $a$, $b$, and $c$ are therefore ``10``,
``1100``, and ``11110000``. In order to meet size requirements, truth
tables can be extended. If, e.g., ``1011`` is the current truth table
in store, the command ``tt -e 3`` extends the truth table to be
defined over 3 variables, yielding ``10111011``.

On can convert truth tables into AIGs using ``convert
--tt_to_aig``. This will construct an AIG in a very naïve way by
constructing each minterm explicitly and then ORing them
all. Conversely, one can obtain truth tables from AIGs using
simulation. For this purpose use the command ``simulate`` with the
flags ``-a`` to simulate from AIGs, ``-t`` to simulate to truth
tables, and ``-n`` to store the simulatuion results. The following
example illustrates the usage for the *c17* benchmark from the ISCAS
benchmark suite. It also employs NPN canonization on the resulting
truth tables using the command ``npn``.

Example
```````

.. code-block:: cirkit

   cirkit> read_aiger c17.aig
   cirkit> simulate -atn
   [i] G16 : 1011100011111000101110001111100010111000111110001011100011111000 (B8F8B8F8B8F8B8F8)
   [i] G17 : 0011001111111111001100001111000000110011111111110011000011110000 (33FF30F033FF30F0)
   [i] runtime: 0.00 secs
   cirkit> store -t
   [i] truth tables in store:
        0: 1011100011111000101110001111100010111000111110001011100011111000
     *  1: 0011001111111111001100001111000000110011111111110011000011110000
   cirkit> current -t 0
   cirkit> npn -t --approach 0
   [i] run-time: 0.89 secs
   [i] NPN class for 1011100011111000101110001111100010111000111110001011100011111000 is 0000000000000000000000000000111111110000111100001111111111111111
   [i] - phase: 1001010 perm: 5 4 1 3 0 2
   cirkit> current -t 1
   cirkit> npn -t --approach 0
   [i] run-time: 0.89 secs
   [i] NPN class for 0011001111111111001100001111000000110011111111110011000011110000 is 0000000000000000000000001111111100001111000011110000111111111111
   [i] - phase: 1001010 perm: 5 0 1 2 4 3

The current truth table in the store corresponds to the last output of
the AIG. Notice that truth table simulation only scales for AIGs with
a small number of inputs. One can obtain a truth table from an
expression using ``convert --expr_to_tt`` or its alias ``expr > tt``.

Some truth table related commands are:

+------------------------------------------------+-----------------------------------------------+
| Command                                        | Description                                   |
+================================================+===============================================+
| ``tt``                                         | Load and modify truth tables                  |
+------------------------------------------------+-----------------------------------------------+
| ``npn``                                        | NPN canonization (exact and heuristic)        |
+------------------------------------------------+-----------------------------------------------+
| ``convert --tt_to_aig``, Alias: ``tt > aig``   | Convert truth table to AIG                    |
+------------------------------------------------+-----------------------------------------------+
| ``convert --expr_to_tt``, Alias: ``expr > tt`` | Convert expression to truth table             |
+------------------------------------------------+-----------------------------------------------+
| ``simulate -atn``                              | Simulates AIGs as truth table and stores them |
+------------------------------------------------+-----------------------------------------------+
| ``simulate -mtn``                              | Simulates MIGs as truth table and stores them |
+------------------------------------------------+-----------------------------------------------+

Expressions
-----------

Expressions provide an easy way to enter Boolean functions into
CirKit. The expressions are multi-level expressions that can contain
constants (``0``, ``1``), Boolean variables (``a``, ``b``, ``c``,
...), inversion (``!``), binary AND (``()``), binary OR (``{}``),
binary XOR (``[]``), and ternary MAJ (``<>``). The whole syntax is
given as follows:

::

     expr ::= 0 | 1 | var | ! expr | ( expr expr ) | { expr expr } | [ expr expr ] | < expr expr expr >
     var ::= a | b | c | ...

Note that ``a`` always corresponds to the least significant bit, ``b``
to the second least significant bit, and so on. Expressions can be
loaded into its store (access flag ``-e``) using the command
``expr``. Expressions can be used as starting point to create truth
tables (``expr > tt``) or binary decision diagrams (``expr > bdd``)
for simple functions and avoid to create a file. The following example
illustrates its usage.

Example
```````

.. code-block:: cirkit

   cirkit> expr (ab)
   cirkit> expr > tt
   cirkit> print -t
   1000
   cirkit> expr !{ac}
   cirkit> expr > tt
   cirkit> print -t
   00000101
   cirkit> expr {{(ab)(ac)}(bc)}
   cirkit> expr > tt
   cirkit> print -t
   11101000
   cirkit> expr
   cirkit> expr > tt
   cirkit> print -t
   11101000

Note that when loading ``!{ac}`` the resulting truth table represents
a 3-variable Boolean function which does not functially depend on the
value for ``b``. The last two examples are both Boolean expressions
for MAJ, the majority-of-three function.

Some commands related to expressions are:

+--------------------------------------------------+-----------------------------------------------+
| Command                                          | Description                                   |
+==================================================+===============================================+
| ``expr``                                         | Load expressions                              |
+--------------------------------------------------+-----------------------------------------------+
| ``convert --expr_to_tt``, Alias: ``expr > tt``   | Convert expression to truth table             |
+--------------------------------------------------+-----------------------------------------------+
| ``convert --expr_to_bdd``, Alias: ``expr > bdd`` | Convert expression to binary decision diagram |
+--------------------------------------------------+-----------------------------------------------+

And-inverter Graphs (AIG)
-------------------------

Loading an AIG into CirKit
``````````````````````````

There are several ways to load an AIG into CirKit. If the AIG is
represented as AIGER file with extension ``*.aig`` if in binary format
and ``*.aag`` if in ASCII format, one can use the command
``read_aiger`` to parse the file and create an AIG in the store. If
already an AIG is in the store, it will be overriden, unless one calls
``read_aiger -n``. If the AIG is represented in Verilog such that
ABC's command ``%read`` is able to parse it, one can use
``read_verilog -a`` to read the Verilog file, convert it into an AIG
and put it into the store. Also BENCH files can be read into AIGs with
the command ``read_bench``. The command ``tt > aig`` allows to
translate the current truth table into an AIG. Internally, ABC's API
will be used for that purpose and the AIG is optimized using ``dc2``.

This summary lists commands to load AIGs into CirKit:

+---------------------------------------------+------------------------------------------------------------+
| Command                                     | Description                                                |
+=============================================+============================================================+
| ``read_aiger``                              | Read AIG from binary or ASCII AIGER file                   |
+---------------------------------------------+------------------------------------------------------------+
| ``read_verilog -a``                         | Read AIG from Verilog file (using ABC's ``%read`` command) |
+---------------------------------------------+------------------------------------------------------------+
| ``read_bench``                              | Read AIG from BENCH file                                   |
+---------------------------------------------+------------------------------------------------------------+
| ``convert -tt_to_aig``, Alias: ``tt > aig`` | Convert truth table to AIG                                 |
+---------------------------------------------+------------------------------------------------------------+

Manipulating the AIG
````````````````````

ABC is a powerful tool for AIG optimization and manipulation and using
the tight integration of CirKit with ABC using the command ``abc``, it
is very easy to use ABC to optimize AIGs in CirKit directly. Hence,
few commands in CirKit exist to perform AIG optimization, but mainly
utility commands.

This list some commands in CirKit to manipulate an AIG:

+----------------+----------------------------------------------------------------------+
| Command        | Description                                                          |
+================+======================================================================+
| ``cone``       | Extracts AIG based on output cones                                   |
+----------------+----------------------------------------------------------------------+
| ``cuts -a``    | Performs cut enumeration                                             |
+----------------+----------------------------------------------------------------------+
| ``propagate``  | Propagates constants through inputs                                  |
+----------------+----------------------------------------------------------------------+
| ``rename``     | Renames inputs and outputs                                           |
+----------------+----------------------------------------------------------------------+
| ``shuffle -a`` | Shuffles I/O of an AIG                                               |
+----------------+----------------------------------------------------------------------+
| ``strash``     | Strashes an AIG (removes dangling nodes)                             |
+----------------+----------------------------------------------------------------------+
| ``unate``      | Computes unateness properties and functional dependencies of the AIG |
+----------------+----------------------------------------------------------------------+

Writing an AIG
``````````````

AIGs can be written into AIGER files using ``write_aiger`` or into Verilog files using ``write_verilog -a``.

This summary lists commands to write AIGs:

+----------------------+-------------------------------------------------------+
| Command              | Description                                           |
+======================+=======================================================+
| ``write_aiger``      | Write AIG to ASCII file (ASCII if suffix is ``.aag``) |
+----------------------+-------------------------------------------------------+
| ``write_verilog -a`` | Write AIG to Verilog file                             |
+----------------------+-------------------------------------------------------+

