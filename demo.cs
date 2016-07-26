# demo.cs
# -------
#
# Shows several usage demos of CirKit. This file
# can be executed with
#
# ./build/programs/cirkit -ef demo.cs
#
# You can call each command with the help option -h
# to get more information about the usage

# make sure to call alias file
< alias

#------------------------------------#
# AIG from truth table               #
#------------------------------------#
# 1. 4-variable truth table (16-bit) #
# 2. Print truth table               #
# 3. Convert truth table to AIG      #
# 4. Show AIG statistics             #
# 5. Write AIG to Verilog            #
# 6. Clear AIG and truth table       #
#------------------------------------#

tt 0xCAFE
print -t
tt > aig
ps -a
write_verilog -a /dev/stdout
store --clear -at
