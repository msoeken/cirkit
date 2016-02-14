# demo.cs
# -------
#
# Shows several usage demos of RevKit. This file
# can be executed with
#
# ./build/programs/revkit -ef addons/cirkit-addon-reversible/demo.cs
#
# You can call each command with the help option -h
# to get more information about the usage



#----------------------------------#
# Transformation based synthesis   #
#----------------------------------#
# 1. TT from permutation           #
# 2. Print truth table             #
# 3. Transf.-based synthesis       #
# 4. Print circuit                 #
# 5. Clear truth table and circuit #
#----------------------------------#

spec -p "0 1 2 4 3 5 6 7"
print -s
tbs
print -c
store --clear -sc


#------------------------------------------#
# Symbolic transformation based synthesis  #
#------------------------------------------#
# 1. Read BDD from PLA                     #
# 2. Embed BDD as RCBDD                    #
# 3. Synthesize circuit from RCBDD         #
# 4. Print circuits                        #
# 5. Show circuit statistics               #
# 6. Show BDD, RCBDD, and circuit in store #
# 7. Clear BDD, RCBDD, and circuit         #
#------------------------------------------#

read_pla ext/functions/cycle10_2_61.pla
embed -b
tbs -b
print -c
ps -c
store -brc
store --clear -brc


#--------------------------------#
# Circuit based synthesis        #
#--------------------------------#
# 1. Create AIG file from string #
# 2. Read AIG                    #
# 3. Show AIG statistics         #
# 4. Synthesize circuit from AIG #
# 5. Show circuit statistics     #
# 6. Clear AIG and circuit       #
# 7. Delete AIG file             #
#--------------------------------#

!echo "aag 11 5 0 2 6\n2\n4\n8\n10\n18\n17\n23\n6 2 4\n12 4 10\n14 8 13\n16 7 15\n20 13 18\n22 15 21\ni0 G1\ni1 G3\ni2 G2\ni3 G4\ni4 G5\no0 G16\no1 G17" > c17.aag
read_aiger c17.aag
ps -a
cbs
ps -c
store --clear -ac
!rm c17.aag

#--------------------------------------#
# Create random circuit                #
#--------------------------------------#
# 1. Creates random circuit with seed  #
# 2. Prints circuit                    #
# 3. Creates truth table from circuit  #
# 4. Performs exact synthesis with SAT #
# 5. Prints circuit                    #
# 6. Show circuit statistics           #
# 7. Clear circuit and truth table     #
#--------------------------------------#

random_circuit --seed 2999860053
print -c
spec -c
exs --mode 1 --new
print -c
ps -c
store --clear -sc
