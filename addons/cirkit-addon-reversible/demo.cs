########################################
# Synthesize circuit from permutation  #
########################################
# 1. TT from permutation               #
# 2. Print truth table                 #
# 3. Transf.-based synthesis           #
# 4. Print circuit                     #
# 5. Clear truth table and circuit     #
########################################

spec -p "0 1 2 4 3 5 6 7"
print -s
tbs
print -c
store --clear -s -c

