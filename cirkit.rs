# commands
alias "rf(.*)" "refactor --strategy=1 -p{}"
alias "rf(.*)" "refactor --strategy=1 -p -z{}"
alias "rw(.*)" "cut_rewrite --strategy=0 -k 4 -p{}"
alias "rw(.*)" "cut_rewrite --strategy=0 -k 4 -p -z{}"
alias "rs(.*)" "resub -p{}"
alias "rsz(.*)" "resub -p -z{}"
alias "bz(.*)" "mighty --area_aware{}"

# flows
alias "compress2" "bz; rw; rf; bz; rw; rwz; bz; rfz; rwz; bz"
alias "compress2rs" "rs --max_pis 6; rw; rs --max_pis 6 --depth 2; rf; rs --max_pis 8; rs --max_pis 8 --depth 2; rw; rs --max_pis 10; rwz; rs --max_pis 10 --depth 2; rs --max_pis 12; rfz; rs --max_pis 12 --depth 2; rwz"
alias "shake" "rw; rf; rfz; rwz; rfz"
