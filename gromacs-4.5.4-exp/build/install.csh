#!/bin/csh -f

cmake ../ -DCMAKE_INSTALL_PREFIX=/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules
make -j8
make install
