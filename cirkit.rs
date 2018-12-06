# commands
alias "rfz(.*)" "refactor --strategy=1 -p -z{}"
alias "rf(.*)" "refactor --strategy=1 -p{}"
alias "rwz(.*)" "cut_rewrite --strategy=0 -k 4 --lutcount 25 -p --multiple -z{}"
alias "rw(.*)" "cut_rewrite --strategy=0 -k 4 --lutcount 25 --multiple -p{}"
alias "rsz(.*)" "resub -p -z{}"
alias "rs(.*)" "resub -p{}"
alias "bz(.*)" "mighty --area_aware{}"

# flows
alias "compress2" "bz; rw; rf; bz; rw; rwz; bz; rfz; rwz; bz"
alias "compress2rs" "bz; rs --max_pis 6; rw; rs --max_pis 6 --depth 2; rf; rs --max_pis 8; bz; rs --max_pis 8 --depth 2; rw; rs --max_pis 10; rwz; rs --max_pis 10 --depth 2; bz; rs --max_pis 12; rfz; rs --max_pis 12 --depth 2; rwz; bz"
alias "shake" "rw; rf; rfz; rwz; rfz"
