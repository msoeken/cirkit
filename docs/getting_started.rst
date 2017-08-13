Getting started
===============

Running CirKit
--------------

CirKit has a command-line interface. After calling
*build/programs/cirkit* a shell prompt is printed to the screen. In
order to see all available commands type ``help`` . This prints a list
with all commands together with a short description for each of
them. To see more details about a command and its usage call the
command together with the ``-h`` flag. For example ``read_aiger -h``
prints options to read an AIGER file.

CirKit can be called in three different modes:

1. **Interactive mode:** This is the default interactive mode that is
   described above.

2. **Bash mode:** ``-c`` In this mode, commands are given to CirKit as
   command line arguments, e.g.,::

     build/programs/cirkit -c "read_aiger file.aig; cone -o y; simulate -at; quit"

   This example reads an AIGER from *file.aig*, reduces the network to
   the cone of output *y*, simulates the resulting network as truth
   table and quits. By adding the flag ``-e`` additionally each
   command is printed to the screen before execution, e.g.,::

     build/programs/cirkit -ec "read_aiger file.aig; cone -o y; simulate -at; quit"

   Note that single character command options (that start eith with a
   single ``-``) can be concatenated, e.g., ``-e -c`` can be written
   as ``-ec``.

3. **Batch mode:** ``-f`` In this mode, commands are read line-by-line
   from a file, e.g.,::

     build/programs/cirkit -f command_file

   This mode also accepts the ``-e`` flag to print each command before
   execution. It is possible to comment some commands in the command
   file by starting a line with a ``#`` character.

Some of the main commands are:

+-----------+---------------------------------------+
| Command   | Description                           |
+===========+=======================================+
| ``alias`` | Creates an alias                      |
+-----------+---------------------------------------+
| ``help``  | Shows all available commands          |
+-----------+---------------------------------------+
| ``quit``  | Quits CirKit                          |
+-----------+---------------------------------------+
| ``set``   | Sets (internal) environment variables |
+-----------+---------------------------------------+

Stores
------

Shared data in CirKit such as circuits or truth tables are stored in
*stores* and commands can access the data from them. Each data
structure has its own store and each store can hold more than one
element. For example, there are seperate stores for truth table, AIGs,
and BDDs. Call ``store -h`` to see all available stores. Each store
comes with its own *command flag* to access it, e.g., ``-a`` for AIGs
and ``-t`` for truth tables. Although a store can hold more than one
element, it is not necessary and possible to specify which store
element to access. Instead each store indivually has a pointer to the
*current store element* and commands always access this one. In order
to access a different store element, one can change the pointer using
the command ``current``. For example, ``current -a 1`` will set the
pointer in the AIG store to the element with index 1 (which is the
second element in the store).

Example
```````

::

     read_aiger file1.aig
     read_aiger file2.aig
     store --show -a
     read_aiger -n file3.aig
     store --show -a
     current -a 0
     store --clear -a

First *file1.aig* is read into the AIG store. The second command reads
*file2.aig* and **overrides** the current entry. Overriding the
current store element is the default behavior of most commands. The
current content of a store can be displayed with ``store --show -a``
or ``store -a`` as a short-hand. In the third line of the example, one
AIG is in the store. When passing the flag ``-n`` to ``read_aiger`` as
in the fourth command a new entry is added to the store and the
current index is updated to the new entry, i.e., at this time the AIG
store contains two elements with the current element being the second
one (index 1). With ``current -a 0`` the current index is reset to 0
and ``store --clear -a`` clears the store from all AIGs.

For many commands it is clear which store they access and it's not
necessary to specify the store. There are some generic commands which
work on all data structures and require to pass the store access flag,
e.g., the command ``store``. The generic commands are:

+-------------+--------------------------------------------------------------------+
| Command     | Description                                                        |
+=============+====================================================================+
| ``convert`` | Converts store elements into other types, e.g., AIGs to BDDs       |
+-------------+--------------------------------------------------------------------+
| ``current`` | Changes the current store pointer                                  |
+-------------+--------------------------------------------------------------------+
| ``print``   | Prints a textual ASCII representation of the current store element |
+-------------+--------------------------------------------------------------------+
| ``ps``      | Prints statistical information about the current store element     |
+-------------+--------------------------------------------------------------------+
| ``show``    | Visualizes the current store element (writes to a *dot* file)      |
+-------------+--------------------------------------------------------------------+
| ``store``   | Shows and clears elements from the store                           |
+-------------+--------------------------------------------------------------------+

Logging
-------

Passing ``-l file.log`` to cirkit creates a log file of the
session. This option is particularly useful in batch mode. The log
file contains a JSON array with an entry for each command. Each entry
contains at least the full command that was run and the time at which
the command was started to execute. Some commands write additional
data into the log file. For example, ``ps -a`` writes number of
inputs, outputs, and AND gates of an AIG, and ``quit`` writes several
information about the computer on which CirKit has been
executed. Being a JSON array, the log file can be easily parsed as
many programming languages have a JSON library.

Some helper functions to parse the log file and, e.g., create ASCII
tables from them can be found in *utils/experiments.py*. Further, the
Python program *utils/extract_script.py* extracts a CirKit script file
from the the log that can be run in batch mode. This can be helpful
when logging an interactive session and then rerunning the commands:

::

     $ cirkit -l session.log
     cirkit> read_aiger file.aig
     cirkit> ps -a
     ...
     cirkit> quit
     $ utils/extract_script.py session.log > session.cs
     $ cirkit -f session.cs

For performing experimental evaluations, the following workflow is
suggested. Create two Python programs (or any other programming
language) called *make_script.py* and *make_table.py*. The program
*make_script.py* writes a CirKit script. The program *make_table.py*
reads the log file created for the script and prints out a table:

::

     $ ./make_script.cs experiments.cs
     $ cirkit -f experiments.cs -l experiments.log
     $ ./make_table.cs experiments.log

Aliases
-------

The command ``alias`` allows to create aliases, which are shortcuts to
commands or sequences of commands. The best place for aliass is the
init file *alias* located in the directory that is specified in the
``$CIRKIT_HOME`` environment variable. It is recommended to set
``$CIRKIT_HOME`` to the root directory of CirKit. Examples for entries
in an alias file are:::

     alias e2t "convert --expr_to_tt"

The alias command gets two arguments, the *key* and the *value* that
is used for substituion. If the key or the value contain a space they
need to be put into quotes, and internal quotes need to be escaped.

Note that they key can be any regular expression with capture groups
and that the value is a formatted string that can contain placeholders
for each capture string: ``%1%`` for the first capture group, ``%2%``
for the second one and so on. Note that the ``%`` sign needs to be
escaped. A more complex example is an alias to read a Verilog file
into an AIG using ABC:::

     alias "abc_verilog (.*)" "abc -c \"%%read %1%; %%blast\""

This will translate, e.g., the command ``abc_verilog file.v`` into::

     abc -c "%read file.v; %blast"

Since the key is any regular expression, we can create aliases which
are very expressive. The alias::

     alias "(\\w+) > (\\w+)" "convert --%1%_to_%2%"

allows, e.g., to convert a truth table into an AIG using ``tt >
aig``. Putting everything together we can write scripts in CirKit such
as::

     abc_verilog file.v
     aig > bdd
     bdd -c

which reads a Verilog file into a CirKit AIG using ABC's API, then
converts the AIG into a BDD and finally computes the characteristic
function of the BDD.

Aliases are also useful inside scripts when they are only required
locally. Consider, e.g., one wants to convert several truth tables
into AIGs, optimize them, and then write them into a file. A script
for this task could look as follows::

     alias "tt_aig_prog ([01]+)" "tt %1%; tt > aig; abc -c &dc2; ps -a; write_aiger %1%.aag"
     tt_aig_prog 11101000
     tt_aig_prog 01011101
     tt_aig_prog 0110
     tt_aig_prog 1001100111010111
     tt_aig_prog 1101110011000000

