# CMake generated Testfile for 
# Source directory: /home/ibrahim/projects/iron-sulfur-project/hybrid-plasma-md/cretin_simulations/Method_paper/molecular_dynamics/GROMACS_hybrid_screening_potential/gromacs-4.5.4-exp
# Build directory: /home/ibrahim/projects/iron-sulfur-project/hybrid-plasma-md/cretin_simulations/Method_paper/molecular_dynamics/GROMACS_hybrid_screening_potential/gromacs-4.5.4-exp/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(TestBuildAll "make")
set_tests_properties(TestBuildAll PROPERTIES  _BACKTRACE_TRIPLES "/home/ibrahim/projects/iron-sulfur-project/hybrid-plasma-md/cretin_simulations/Method_paper/molecular_dynamics/GROMACS_hybrid_screening_potential/gromacs-4.5.4-exp/CMakeLists.txt;830;add_test;/home/ibrahim/projects/iron-sulfur-project/hybrid-plasma-md/cretin_simulations/Method_paper/molecular_dynamics/GROMACS_hybrid_screening_potential/gromacs-4.5.4-exp/CMakeLists.txt;0;")
subdirs("share")
subdirs("include")
subdirs("man")
subdirs("src")
subdirs("scripts")
subdirs("tests")
