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
store --clear -s -c


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
store -b -r -c
store --clear -b -r -c


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
store --clear -s -c
