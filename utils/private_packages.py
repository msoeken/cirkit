class package_bds:
    description = "BDD Decomposition package"
    subdir      = "bds-1-2.15"
    url         = "http://www.informatik.uni-bremen.de/revkit/files/bds-1-2.15.tar.gz"
    fmt         = "tar-gz"
    build       = [ "make all" ]
    install     = [ "make install" ]

# This does not compile yet
class package_atom:
    description = "A hackable text editor for the 21st Century"
    subdir      = "atom"
    url         = "https://github.com/atom/atom"
    fmt         = "git"
    build       = [ "export PYTHON=/usr/bin/python2", "git fetch -p", "git checkout $(git describe --tags `git rev-list --tags --max-count=1`)", "script/build" ]
    install     = [ "cp -v $TMPDIR/atom-build/Atom/atom %s/atom", "cp -v $TMPDIR/atom-build/Atom/apm %s/apm" ]
