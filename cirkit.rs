# commands
alias "rf(.*)" "refactor --strategy=1 -p{}"
alias "rf(.*)" "refactor --strategy=1 -p -z{}"
alias "rw(.*)" "cut_rewrite --strategy=0 -k 4 -p{}"
alias "rw(.*)" "cut_rewrite --strategy=0 -k 4 -p -z{}"
alias "rs(.*)" "resub -p{}"
alias "rsz(.*)" "resub -p -z{}"

# flows
alias "compress2" "rw; rf; rw; rwz; rfz; rwz"
alias "compress2rs" "rs --max_pis 6; rw; rs --max_pis 6 --depth 2; rf; rs --max_pis 8; rs --max_pis 8 --depth 2; rw; rs --max_pis 10; rwz; rs --max_pis 10 --depth 2; rs --max_pis 12; rfz; rs --max_pis 12 --depth 2; rwz"
