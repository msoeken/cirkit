class package_bds:
    description = "BDD Decomposition package"
    subdir      = "bds-1-2.15"
    url         = "http://www.informatik.uni-bremen.de/revkit/files/bds-1-2.15.tar.gz"
    fmt         = "tar-gz"
    build       = [ "make all" ]
    install     = [ "make install" ]
