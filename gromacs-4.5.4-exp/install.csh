#!/bin/csh -f

cmake ../ -DCMAKE_INSTALL_PREFIX=/home/ibrahim/programs/test
make -j8
make install
