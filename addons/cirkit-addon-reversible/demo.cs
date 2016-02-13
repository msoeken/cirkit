#--------------------------------------#
# Synthesize circuit from permutation  #
#--------------------------------------#
# 1. TT from permutation               #
# 2. Print truth table                 #
# 3. Transf.-based synthesis           #
# 4. Print circuit                     #
# 5. Clear truth table and circuit     #
#--------------------------------------#

spec -p "0 1 2 4 3 5 6 7"
print -s
tbs
print -c
store --clear -s -c


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
