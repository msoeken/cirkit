RevKit
======

This page lists some RevKit specific documentation.

Adding a command to RevKit
--------------------------

This tutorial explains how to integrate a new simple command into
RevKit. As an example, a command called `unopt` is implemented, that
copies gate in a reversible circuit without modifying the
functionality.

CirKit provides utility scripts in order to create new files
easily. We create a file for the new command using the following
script::

  ./utils/make_src_file.py cli/commands/unopt reversible

Note, that the first argument is the path to the filename without the
*src/* in the beginning and without an extension in the end. Two
files, a header and a source file, are created. The second parameter
ensures that the files are created for the RevKit add-on. The third
parameter is optional and can have a name of the file author. If not
specified, the name is fetched from the user's git configuration.

The header file in
*addons/cirkit-addon-reversible/src/cli/commands/unopt.hpp* contains
already some skeleton code and we extend it as follows (all comments
are omitted in the code):

.. code-block:: c++

   // unopt.hpp
   #ifndef CLI_UNOPT_COMMAND_HPP
   #define CLI_UNOPT_COMMAND_HPP

   #include <cli/cirkit_command.hpp>

   namespace cirkit
   {

   class unopt_command : public cirkit_command
   {
   public:
     unopt_command( const environment::ptr& env );
     rules_t validity_rules() const;

   protected:
     bool execute();

   private:
     unsigned copies = 1u;
   };

   }

   #endif

We define a command with the base class ``cirkit_command``. It is
important that the class name is ``unopt_command`` to be used in later
code that makes use of macros and relies on some naming
conventions. Two methods need to be implemented, the constructor that
will set up the arguments which can be passed to the command, and
``execute`` which executes the code and calls our algorithm. We also
implement the method ``validity_rules`` to ensure that the store
contains at least one reversible circuit when calling the
command. More details on how to write commands can be found in the
abc_cli_ example program.

.. _abc_cli: https://github.com/msoeken/cirkit/blob/master/programs/core/abc_cli.cpp

.. code-block:: c++

   // unopt.cpp
   #include "unopt.hpp"

   #include <alice/rules.hpp>
   #include <cli/reversible_stores.hpp>
   #include <core/utils/program_options.hpp>
   #include <reversible/target_tags.hpp>
   #include <reversible/functions/copy_metadata.hpp>

   namespace cirkit
   {

   unopt_command::unopt_command( const environment::ptr& env )
     : cirkit_command( env, "unoptimize circuits" )
   {
     opts.add_options()
       ( "copies,c", value_with_default( &copies ), "number of gate copies" )
       ;

     add_new_option(); /* adds a flag --new, or -n that can be used to add a new
                          store entry instead of overwriting it */
   }

   command::rules_t unopt_command::validity_rules() const
   {
     return {has_store_element<circuit>( env )};
   }

   bool unopt_command::execute()
   {
     auto& circuits = env->store<circuit>(); /* access store with reversible circuits */

     /* reference to current circuit, and new circuit with same properties */
     const auto& circ = circuits.current();
     circuit circ_new;
     copy_metadata( circ, circ_new );

     for ( const auto& g : circ )     /* iterate through the gates */
     {
       circ_new.append_gate() = g;    /* copy existing gate */
       if ( is_toffoli( g ) )         /* some more copies, if gate is Toffoli */
       {
         for ( auto i = 0u; i < 2u * copies; ++i )
         {
           circ_new.append_gate() = g;
         }
       }
     }

     extend_if_new( circuits ); /* extend store by empty element if --new option is set */
     circuits.current() = circ_new;

     return true; /* always return true */
   }

   }

The function should always return ``true``.

We are almost done. Next, we add the command to the RevKit
executable. For this purpose, open the file
*addons/cirkit-addon-reversible/programs/reversible/revkit.cpp* and
add the following header, where other headers are included:

.. code-block:: c++

   #include <cli/commands/unopt.hpp>

And then add the command in the same style as other commands are added using:

.. code-block:: c++

   ADD_COMMAND( unopt );

That's it. We rebuild RevKit with::

     make -C build revkit

and then call it to try out the new command:

.. code-block:: cirkit

   revkit> read_spec -p "0 4 2 1 0 3 7 5"
   revkit> tbs
   [i] run-time: 0.00 secs
   revkit> ps -c
   Lines:        3
   Gates:        7
   T-count:      21
   Logic qubits: 4
   revkit> unopt
   revkit> ps -c
   Lines:        3
   Gates:        21
   T-count:      63
   Logic qubits: 4

Exercises
`````````

Here are some suggestions for exercises (with a difficulty estimation from 0–50) to extend the add-on.

1. [25] Copy all gates which are self-inverse in this manner based on a syntactic comparison.

