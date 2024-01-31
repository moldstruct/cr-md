# Hybrid Collisional-Radiative/Molecular Dynamics

This is a Hybrid Collisional-Radiative/Molecular Dynamics (CR/MD) model that is part of the MOLDSTRUCT toolbox. It can be used to simulate photon-matter interaction for large systems such as large macromolecules, protein crystals and liquids. The code is based on a modified version of GROMACS version 4.5 ([webpage](https://www.gromacs.org/)). With functionality close to the normal GROMACS but with capability of running large-scale photon-matter simulations. Running the code requires data regarding the charge distribution, free electron temperature/density and optionally electron-ion coupling. The model is developed by the Biophysics group at Uppsala University and has a preprint here: https://arxiv.org/abs/2401.03180.

To run photon-matter interaction calculations, the following files are needed:

**IONIZATION_DATA** folder located in the same directory as the simulations is run.
This folder should contain:
1. a folder called **charge_data** which contains files named **charge_distribution_timestep_x.txt** 
2. files **debye_shielding_interpolated_x.txt**, **electron_density_x.txt** and **electron_temperature_x.txt**.
4. A file called **lennard_jones_parameters.txt** which contains LJ data for each unique atom type given by GROMACS topology file (.tpr). 

Note that **x** is data for the simulation step **x**.

For the file, **charge_distribution_timestep_x.txt**, it should correspond to a list with the net-charge for each atom in the system. The order of charge should be set in the same way as how the atoms are listed in the PDB file. Thus, the first value in the file should correspond to the net-charge of the first atom in the PDB for time-step **x**. 

For the files in 2. a single value for each quantity should be located in the file corresponding to the value for time-step **x**. 

For 3. each line the has three numbers for the LJ parameters: the atomic number **Z**, **epsilon**, **sigma**

This could be needed if one wants to change the values of epsilon and sigma as the atom is ionized. However, this is not always necessary and can be set to zero. In this case, a simple factor scales down the LJ interaction based on the charge of the atom and the atomic number Z.

In the Example folder, all files above are available required to run a photon-matter calculation of a Lysozyme crystal as done in the preprint. 

