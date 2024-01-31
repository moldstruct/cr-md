gmx='/path/to/install/location/bin' # path to where GROMACS is installed

filename="conf.gro"
idx=1

$gmx/grompp -f full_sim.mdp -po mdout_$idx -c $filename -n index.ndx -p ./topology/topol.top -o output_$idx.tpr -maxwarn 3  # create .tpr file

$gmx/mdrun -s output_$idx.tpr -v -deffnm output_$idx # run simulation, use -nt X, where X is number of cores you want to run with specific number of cores

rm MPI_slice_n*
