import os
import sys
import numpy as np


def get_number_of_atom_species():

	""" Function to extract the unique atom types as given by GROMACS' tpr file which
		was dumped to the file "out". This can be done using GROMACS' gmxdump command. """

	f = open("./out", "r")
	lines = f.readlines()
	atom_number = []
	atom_names = []
	types = []

	for i, line in enumerate(lines):

		if "ptype" in line and "Atom" in line:
		
			if int(line.split()[2].split(",")[0]) not in types:
				types.append(int(line.split()[2].split(",")[0]))
			else:
				continue

			atomnum = int(line.split()[len(line.split())-1].split("}")[0])
			atom_number.append(int(line.split()[len(line.split())-1].split("}")[0]))					
	
		if "atom" in line and "name=" in line:
			a = line.split("=")[2].split("}")[0].replace('"', '')
			atom_names.append(a)

	f.close()
	return types, atom_number, atom_names
			
types, atom_number, atom_names = get_number_of_atom_species()

# # write data to the Lennard Jones file. Note that we set the LJ parameters epislon and sigma to 0.

f = open("lennard_jones_parameters.txt", "w") 

for i, j in zip(types, atom_number):
	f.writelines("{0} 0 0\n".format(j))
f.close()






