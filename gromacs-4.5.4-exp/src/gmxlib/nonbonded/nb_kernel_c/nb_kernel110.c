/*
 *                This source code is part of
 *
 *                 G   R   O   M   A   C   S
 *
 * Copyright (c) 1991-2000, University of Groningen, The Netherlands.
 * Copyright (c) 2001-2009, The GROMACS Development Team
 *
 * Gromacs is a library for molecular simulation and trajectory analysis,
 * written by Erik Lindahl, David van der Spoel, Berk Hess, and others - for
 * a full list of developers and information, check out http://www.gromacs.org
 *
 * This program is free software; you can redistribute it and/or modify it under 
 * the terms of the GNU Lesser General Public License as published by the Free 
 * Software Foundation; either version 2 of the License, or (at your option) any 
 * later version.
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU Lesser General Public License.  
 *
 * In plain-speak: do not worry about classes/macros/templates either - only
 * changes to the library have to be LGPL, not an application linking with it.
 *
 * To help fund GROMACS development, we humbly ask that you cite
 * the papers people have written on it - you can find them on the website!
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>

#include "vec.h"
#ifdef GMX_THREAD_SHM_FDECOMP
#include "thread_mpi.h"
#endif

#include "nb_kernel110.h"
#include "../nb_kerneltype.h"

/*
 * Gromacs nonbonded kernel nb_kernel110
 * Coulomb interaction:     Normal Coulomb
 * VdW interaction:         Lennard-Jones
 * water optimization:      No
 * Calculate forces:        yes
 */
extern double debye_length;
extern double electron_density;
double *epsilon_array;
double *sigma_array;
int *ATOM_Z;

double poly3(double p1, double p2, double p3, double p4, double x)
{
    return p1 * pow(x, 3.0) + p2 * pow(x, 2.0) + p3 * x + p4;
};

double poly1(double p1, double p2, double x)
{
    return (x == 0.0) ? 0.0 : p1 * x + p2;
};

double scaleRmin(int Z, double charge)
{
	double Z_double = (double)Z;
    double e = Z_double - charge;
    double p1 = 0.0;
    double p2 = 0.0;
    double p3 = 0.0;
    double p4 = 0.0;

    switch (Z)
    {

 	case 1:
        p1 = 0.0;
        p2 = 0.0;
        p3 = 0.0;
        p4 = 1.0;
        break;
    case 6:
        p1 = 0.004985;
        p2 = -0.04608;
        p3 = 0.2332;
        p4 = 0.1831;
        break;
    case 7:
        p1 = 0.004306;
        p2 = -0.04864;
        p3 = 0.2493;
        p4 = 0.161;
        break;
    case 8:
        p1 = 0.0033;
        p2 = -0.04291;
        p3 = 0.2366;
        p4 = 0.1633;
        break;
    case 11:
        p1 = 0.002899;
        p2 = -0.04709;
        p3 = 0.2532;
        p4 = 0.05435;
        break;
    case 16:
        p1 = 0.0004371;
        p2 = -0.008512;
        p3 = 0.07822;
        p4 = 0.2666;
        break;
    case 17:
        p1 = 0.000274;
        p2 = -0.005513;
        p3 = 0.05835;
        p4 = 0.2547;
        break;
    }
    return poly3(p1, p2, p3, p4, e);
};

double scaleEpsilon(int Z, double charge)
{

	double Z_double = (double)Z;
    double e = Z_double - charge;
    double p1 = 0.0;
    double p2 = 0.0;
    switch (Z)
    {
	case 1:
        p1 = 0.0;
        p2 = 1.0;
        break;
    case 6:
        p1 = 0.1096;
        p2 = 0.3422;
        break;
    case 7:
        p1 = 0.09189;
        p2 = 0.3567;
        break;
    case 8:
        p1 = 0.079;
        p2 = 0.3679;
        break;
    case 11:
        p1 = 0.06471;
        p2 = 0.2822;
        break;
    case 16:
        p1 = 0.04305;
        p2 = 0.3499;
        break;
    case 17:
        p1 = 0.04093;
        p2 = 0.2997;
        break;
    }
    return poly1(p1, p2, e);
};

// Below is the kernel computation - the actual calculation of non-bonded interactions.

void nb_kernel110(
                    int *           p_nri,
                    int *           iinr,
                    int *           jindex,
                    int *           jjnr,
                    int *           shift,
                    real *          shiftvec,
                    real *          fshift,
                    int *           gid,
                    real *          pos,
                    real *          faction,
                    real *          charge,
                    real *          p_facel,
                    real *          p_krf,
                    real *          p_crf,
                    real *          Vc,
                    int *           type,
                    int *           p_ntype,
                    real *          vdwparam,
                    real *          Vvdw,
                    real *          p_tabscale,
                    real *          VFtab,
                    real *          invsqrta,
                    real *          dvda,
                    real *          p_gbtabscale,
                    real *          GBtab,
                    int *           p_nthreads,
                    int *           count,
                    void *          mtx,
                    int *           outeriter,
                    int *           inneriter,
                    real *          work)
{
    int           nri,ntype,nthreads;
    real          facel,krf,crf,tabscale,gbtabscale;
    int           n,ii,is3,ii3,k,nj0,nj1,jnr,j3,ggid;
    int           nn0,nn1,nouter,ninner;
    real          shX,shY,shZ;
    real          fscal,tx,ty,tz;
    real          rinvsq, r;
    real          iq;
    real          qq,vcoul,vctot;
    int           nti;
    int           tj;
    real          rinvsix;
    real          Vvdw6,Vvdwtot;
    real          Vvdw12;
    real          ix1,iy1,iz1,fix1,fiy1,fiz1;
    real          jx1,jy1,jz1;
    real          dx11,dy11,dz11,rsq11,rinv11;
    real          c6,c12;

    nri              = *p_nri;         
    ntype            = *p_ntype;       
    nthreads         = *p_nthreads;    
    facel            = *p_facel;       
    krf              = *p_krf;         
    crf              = *p_crf;         
    tabscale         = *p_tabscale;  
	double sigmas;
	double epsilons;
	double sigma6;
    float rmin;
	double sigma1;
	double epsilon1;
	double sigma2;
	double epsilon2;
	double r_times_debye_inverse;
	double sigma3;
	double vdW_force_check;
	double morse_potential;
	double morse_force;
	double beta = 20.16562;
	int testing = 2;
	int hybrid_potential = 1;
	int lj_change = 0;
	double LJ_factor;
	double LJ_factor_epsilon;
	double LJ_factor_sigma;
	double RMIN;

	double lj_c6_factor;
	double lj_c12_factor;
	real lj4_factor = work[3];

    /* Reset outer and inner iteration counters */
    nouter           = 0;              
    ninner           = 0;              

    /* Loop over thread workunits */

// printf("c6[i]= %lf,c12[i]= %lf, ATOM_Z[i]= %i \n",sigma_array[0],epsilon_array[0],ATOM_Z[0]);
// printf("c6[i]= %lf,c12[i]= %lf, ATOM_Z[i]= %i \n",sigma_array[1],epsilon_array[1],ATOM_Z[1]);

	//exit(0);
	//debye_length = 1000;
	//debye_length = 0.1;
	double debye_inverse = 1/debye_length;
    
    do
    {
#ifdef GMX_THREAD_SHM_FDECOMP
        tMPI_Thread_mutex_lock((tMPI_Thread_mutex_t *)mtx);
        nn0              = *count;         
		
        /* Take successively smaller chunks (at least 10 lists) */
        nn1              = nn0+(nri-nn0)/(2*nthreads)+10;
        *count           = nn1;            
        tMPI_Thread_mutex_unlock((tMPI_Thread_mutex_t *)mtx);
        if(nn1>nri) nn1=nri;
#else
	    nn0 = 0;
		nn1 = nri;
#endif
        /* Start outer loop over neighborlists */

		//printf("Hi im in nb_kernel110\n");
        //printf("DEBYE LENGTH IN nbkernel110.c: %lf\n", debye_length);
        
        for(n=nn0; (n<nn1); n++)
        {

            /* Load shift vector for this list */
            is3              = 3*shift[n];     
            shX              = shiftvec[is3];  
            shY              = shiftvec[is3+1];
            shZ              = shiftvec[is3+2];

            /* Load limits for loop over neighbors */
            nj0              = jindex[n];      
            nj1              = jindex[n+1];    

            /* Get outer coordinate index */
            ii               = iinr[n];        
            ii3              = 3*ii;           

            /* Load i atom data, add shift vector */
            ix1              = shX + pos[ii3+0];
            iy1              = shY + pos[ii3+1];
            iz1              = shZ + pos[ii3+2];

            /* Load parameters for i atom */
            iq               = facel*(charge[ii]);
            nti              = 2*ntype*type[ii];

            /* Zero the potential energy for this list */
            vctot            = 0;              
            Vvdwtot          = 0;              

            /* Clear i atom forces */
            fix1             = 0;              
            fiy1             = 0;              
            fiz1             = 0;      


            /* Calculate new sigma and epsilon based on charge and atomic number Z */
			sigma1 = 1 ; // (sigma_array[type[ii]])*scaleRmin(ATOM_Z[type[ii]], floor(charge[ii]));
			epsilon1 = 1; // (epsilon_array[type[ii]])* scaleEpsilon(ATOM_Z[type[ii]], floor(charge[ii]));
			morse_potential=0;
			morse_force = 0;

			double c0 = 0;
			double c1 = 0;
			double c3 = 0;
			double rprime = 0;
			double rs = 0;
			//double ne = 1.0075413666666668e+23;
			double krs_cube;	
			double kurs_cube_plus_one;
			double k_square_rs_cube;




			//charge[ii] = 1.0;

			rs = 0.01*1e9*pow((3*charge[ii]/(electron_density*4*3.14159)), 1.0/3.0);
			krs_cube = debye_inverse*rs*debye_inverse*rs*debye_inverse*rs;//pow(debye_inverse*rs, 3.0);
			kurs_cube_plus_one = krs_cube+1.0;
			k_square_rs_cube= debye_inverse*debye_inverse*rs*rs*rs;

			c0 = -1;
			c1 = ((3)/(2*k_square_rs_cube))*(pow(kurs_cube_plus_one,(2.0/3.0))-1);
			rprime = (1/debye_inverse)*(pow(pow(debye_inverse*rs, 3.0)+1, 1.0/3.0)-1);
			c3 = -((3)/(k_square_rs_cube))*rprime*exp(debye_inverse*rprime);

			//printf("c0=%lf, c1= %lf, c3= %lf, rprime= %lf, rs= %lf, debye_inverse = %lf debye_length = %lf, k*rs= %lf, zeff = %lf, charge[ii] = %lf \n", c0, c1, c3, rprime, rs, debye_inverse, debye_length, debye_inverse*rs, rprime*((3)/(k_square_rs_cube)), charge[ii]);         

            for(k=nj0; (k<nj1); k++)
            {

		 		//printf("MASST = mdatoms->massT[0]= %lf \n", MASS[jnr]);
        		//printf("MASST = mdatoms->massT[1]= %lf \n", MASS[ii]);
           		//printf("MASST = mdatoms->massT[2]= %lf \n", MASS[2]);


                /* Get j neighbor index, and coordinate index */
                jnr              = jjnr[k];        
                j3               = 3*jnr;      


				// if (charge[ii] < 0.5 ||  charge[jnr] <0.5) { 

				//	continue;
		
				//	}    

                /* load j atom coordinates */
                jx1              = pos[j3+0];      
                jy1              = pos[j3+1];      
                jz1              = pos[j3+2];      

                /* Calculate distance */
                dx11             = ix1 - jx1;      
                dy11             = iy1 - jy1;      
                dz11             = iz1 - jz1;      
                rsq11            = dx11*dx11+dy11*dy11+dz11*dz11;

                /* Calculate 1/r and 1/r2 */
                rinv11           = gmx_invsqrt(rsq11);
				r 				 = rsq11*rinv11;
				//if (r> 1.1) {
			//		continue;
			//	}

                /* Load parameters for j atom */
                qq               = iq*(charge[jnr]); 
                tj               = nti+2*type[jnr];
                c6               = vdwparam[tj];   
                c12              = vdwparam[tj+1]; 
                rinvsq           = rinv11*rinv11;

				r_times_debye_inverse = r*debye_inverse; // we do this calculation multiple times, so save it in a variable 

				if (hybrid_potential==0) { 
                /* Coulomb interaction */
                //vcoul            = qq*rinv11;
				r_times_debye_inverse = r*debye_inverse; // we do this calculation multiple times, so save it in a variable 

				vcoul 			   = qq*rinv11*exp((-r_times_debye_inverse));    //qq*rinv11*exp((-r)*debye_inverse);   
				//vcoul            = qq*rinv11*((r + debye_length)/(debye_length))*exp((-r)/debye_length);   
				//fscal            = vcoul*rinvsq*((r + debye_length)/(debye_length));           
                vctot            = vctot+vcoul;  

				}

				else {
			
					/* Hybrid ion-sphere/debye screening model*/

					//c0 = c0*qq;
					//c1 = c1*qq;
					//c3 = c3*qq;

		
						
					//c1 = (3*charge[ii])/(2*debye_inverse*debye_inverse*rs*rs*rs)

					if (charge[ii] < 0.5 ||  charge[jnr] <0.5) { 
				 
					if ( r <= rprime)  {
						vcoul 			 = -((c0*qq*rinv11) + c1*qq - ((qq*r*r)/(2*pow(rs,3.0))))     ;  //qq*rinv11*exp((-r_times_debye_inverse));    //qq*rinv11*exp((-r)*debye_inverse);   
						if (isnan(vcoul)) { 
						printf("Nan: Vcoul= %lf, c0= %lf, c1= %lf, qq = %lf, r= %lf, rs=%lf\n", vcoul, c0, c1, qq, r, rs);
								exit(0);
						} 
						//printf("Vcoul= %lf, c0= %lf, c1= %lf, qq = %lf, r= %lf, rs=%lf\n", vcoul, c0, c1, qq, r, rs);
				        vctot            = vctot+vcoul;   
					}
					else {

						vcoul 			 = -(c3*qq*exp(-r_times_debye_inverse))     ;  //qq*rinv11*exp((-r_times_debye_inverse));    //qq*rinv11*exp((-r)*debye_inverse);  
						if (isnan(vcoul)) { 
						printf("Nan Vcoul= %lf, c0= %lf, c1= %lf, qq = %lf, r= %lf, rs=%lf\n", vcoul, c0, c1, qq, r, rs);
								exit(0);
						} 
						//printf("Vcoul= %lf, c0= %lf, c1= %lf, qq = %lf, r= %lf, rs=%lf\n", vcoul, c0, c1, qq, r, rs);
				        vctot            = vctot+vcoul;   

					}
					}
					else {
						vcoul = 0; 
				        vctot            = vctot+vcoul;   

					}

				}
		

				
				morse_potential = 0.0;
				morse_force = 0.0;
		  
				if (lj_change == 1 ) {
		
            	/* Calculate new sigma and epsilon for Lennard Jones interaction based on charge and atomic number Z */
				//printf("Type ii : %i, type[jnr]: %i", )

				if (testing == 3) {


				if ( (ATOM_Z[type[jnr]]==1 && charge[jnr] == 1 && charge[ii] >= 1.0 ) || (ATOM_Z[type[ii]]==1 && charge[ii] ==1 && charge[jnr] >= 1.0))	{ // could be && instead of ||
					// if at least one of the atoms is hydrogen, and its charge is +1, then we do not compute LJ interaction 

					c6 = 0;
					c12 = 0;
					//vdwparam[tj] = 0;
					//vdwparam[tj+1] = 0;
				}
				else {
					if ( (ATOM_Z[type[ii]]==1) && (ATOM_Z[type[jnr]]==1) ) {

					// Calculate new sigma and epsilon based on charge and atomic number Z */
						sigma1 = (sigma_array[type[ii]]); //*scaleRmin(ATOM_Z[type[ii]], floor(charge[ii]));
						epsilon1 = (epsilon_array[type[ii]]); // * scaleEpsilon(ATOM_Z[type[ii]], floor(charge[ii]));
						sigma2 = (sigma_array[type[jnr]]); //*scaleRmin(ATOM_Z[type[jnr]], floor(charge[jnr]));
						epsilon2 = (epsilon_array[type[jnr]]); //* scaleEpsilon(ATOM_Z[type[jnr]], floor(charge[jnr]));
						sigmas = 0.5*(sigma1 + sigma2);
						epsilons = pow(epsilon1*epsilon2, 0.5);

					}
					else if ( (ATOM_Z[type[ii]]==1) ) {
						sigma1 = (sigma_array[type[ii]]); //*scaleRmin(ATOM_Z[type[ii]], floor(charge[ii]));
						epsilon1 = (epsilon_array[type[ii]]); // * scaleEpsilon(ATOM_Z[type[ii]], floor(charge[ii]));
						sigma2 = (sigma_array[type[jnr]])*scaleRmin(ATOM_Z[type[jnr]], floor(charge[jnr]));
						epsilon2 = (epsilon_array[type[jnr]])* scaleEpsilon(ATOM_Z[type[jnr]], floor(charge[jnr]));
						sigmas = 0.5*(sigma1 + sigma2);
						epsilons = pow(epsilon1*epsilon2, 0.5);


					}
					else if ( ATOM_Z[type[jnr]]==1) {
						sigma1 = (sigma_array[type[ii]])*scaleRmin(ATOM_Z[type[ii]], floor(charge[ii]));
						epsilon1 = (epsilon_array[type[ii]])* scaleEpsilon(ATOM_Z[type[ii]], floor(charge[ii]));
						sigma2 = (sigma_array[type[jnr]]);      // *scaleRmin(ATOM_Z[type[jnr]], floor(charge[jnr]));
						epsilon2 = (epsilon_array[type[jnr]]);    // * scaleEpsilon(ATOM_Z[type[jnr]], floor(charge[jnr]));
						sigmas = 0.5*(sigma1 + sigma2);
						epsilons = pow(epsilon1*epsilon2, 0.5);

					}

					else {
						sigma1 = (sigma_array[type[ii]])*scaleRmin(ATOM_Z[type[ii]], floor(charge[ii]));
						epsilon1 = (epsilon_array[type[ii]])* scaleEpsilon(ATOM_Z[type[ii]], floor(charge[ii]));
						sigma2 = (sigma_array[type[jnr]])*scaleRmin(ATOM_Z[type[jnr]], floor(charge[jnr]));
						epsilon2 = (epsilon_array[type[jnr]])* scaleEpsilon(ATOM_Z[type[jnr]], floor(charge[jnr]));
						sigmas = 0.5*(sigma1 + sigma2);
						epsilons = pow(epsilon1*epsilon2, 0.5);

					}
	

				}
				
				//sigma6 = pow(sigmas, 6.0);
				sigma3 = sigmas*sigmas*sigmas;
			    sigma6 = sigma3*sigma3;//sigmas*sigmas*sigmas*sigmas*sigmas*sigmas;
				c6 = 4*epsilons*sigma6;
				c12 = c6*sigma6; // 4*epsilons*sigma6*sigma6; // write c12 = c6*sigma6
				
				}

				if (testing == 2) {
			
				if ( (ATOM_Z[type[jnr]]==1 && charge[jnr] == 1 && charge[ii] >= 1.0 ) || (ATOM_Z[type[ii]]==1 && charge[ii] ==1 && charge[jnr] >= 1.0))	{ // could be && instead of ||
					// if at least one of the atoms is hydrogen, and its charge is +1, then we do not compute LJ interaction 

					c6 = 0;
					c12 = 0;
					//vdwparam[tj] = 0;
					//vdwparam[tj+1] = 0;
				}
			
				else {
				/* Calculate new sigma and epsilon based on charge and atomic number Z */
				//sigma1 = (sigma_array[type[ii]])*scaleRmin(ATOM_Z[type[ii]], floor(charge[ii]));
				//epsilon1 = (epsilon_array[type[ii]])* scaleEpsilon(ATOM_Z[type[ii]], floor(charge[ii]));
	
				if (type[ii] == type[jnr] && charge[ii]==charge[jnr]) {
	
					sigmas = sigma1;
					epsilons = epsilon1;

				}

				else {
					// lets do Morse instead!:
					//bij = 0.1     345000
					//D = 463


					if ( (ATOM_Z[type[jnr]]==8 && ATOM_Z[type[ii]]==1) || (ATOM_Z[type[ii]]==8 && ATOM_Z[type[jnr]]==1)) {
					// if oxygen + hydrogen
					//morse_potential = 463*(1-exp(-345000*(r-0.1)))*(1-exp(-345000*(r-0.1)));
					//morse_force = 2*463*345000*(1-exp(-345000*(r-0.1)))*(exp(-345000*(r-0.1)))*rinv11;
					//sigmas = 0;
					//epsilons = 0;



					//printf("Calculating bonded with LJ!!\n");
					sigma2 = (sigma_array[type[jnr]])*scaleRmin(ATOM_Z[type[jnr]], floor(charge[jnr]));
					epsilon2 = (epsilon_array[type[jnr]])* scaleEpsilon(ATOM_Z[type[jnr]], floor(charge[jnr]));

					//sigmas = 0.519615*0.5*(sigma1 + sigma2);
					sigmas = 0.5*(sigma1 + sigma2);
					epsilons = pow(epsilon1*epsilon2, 0.5);
					//epsilons = epsilons; // this is the interaction between O-H in the water molecule, we scale rmin to get the bond length - otherwise things will blow up!

					}
					else {
		
					sigma2 = (sigma_array[type[jnr]])*scaleRmin(ATOM_Z[type[jnr]], floor(charge[jnr]));
					epsilon2 = (epsilon_array[type[jnr]])* scaleEpsilon(ATOM_Z[type[jnr]], floor(charge[jnr]));

					sigmas = 0.5*(sigma1 + sigma2);
					epsilons =pow(epsilon1*epsilon2, 0.5);

					
					}

				}
				
				//sigma6 = pow(sigmas, 6.0);
				sigma3 = sigmas*sigmas*sigmas;
			    sigma6 = sigma3*sigma3;//sigmas*sigmas*sigmas*sigmas*sigmas*sigmas;
				c6 = 4*epsilons*sigma6;
				c12 = c6*sigma6; // 4*epsilons*sigma6*sigma6; // write c12 = c6*sigma6

				}
				}

				//c6               = vdwparam[tj];  
                //c12              = vdwparam[tj+1];



				if (testing == 1) {
				//printf("Testing!! \n");
				if ( (ATOM_Z[type[jnr]]==8 && ATOM_Z[type[ii]]==1) || (ATOM_Z[type[ii]]==8 && ATOM_Z[type[jnr]]==1)) {

					//if  ((abs(jnr-(ii+1))<1e-5) || (abs(jnr-(ii+2))<1e-5)) {
					if  (((jnr==(ii+1)) && (ATOM_Z[type[ii]]==8)) || ((jnr==(ii+2)) && (ATOM_Z[type[ii]]==8))) {

						 //int rank, size, kk;
   						//MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    					//MPI_Comm_size(MPI_COMM_WORLD, &size);
						//printf("We are from process: %i\n", rank);

						//printf("ii: %i jnr: %i, r= %lf \n",ii,jnr, r);
				 		morse_potential = 463*(1-exp(-beta*(r-0.09572)))*(1-exp(-beta*(r-0.09572)));
						morse_force = -2*463*beta*(1-exp(-beta*(r-0.09572)))*(exp(-beta*(r-0.09572)))*rinv11;
						c6 = 0;
						c12 = 0;

					}
					else {

					sigma2 = (sigma_array[type[jnr]]);//*scaleRmin(ATOM_Z[type[jnr]], floor(charge[jnr]));
					epsilon2 = (epsilon_array[type[jnr]]);// * scaleEpsilon(ATOM_Z[type[jnr]], floor(charge[jnr]));

					//sigma1 = 0.302905564168;
					//sigma2 = 0.0400013524445;
					//sigmas = 0.4803532895066994530034626386310031615396196115822753048627490339*0.5*(sigma1 + sigma2);
					sigmas = 0.5*(sigma1 + sigma2);
					//epsilon1 = 0.50208;
					//epsilon2 = 0.192464;
					//sigmas = 0.40*0.5*(sigma1 + sigma2);
					epsilons = pow(epsilon1*epsilon2, 0.5);
					epsilons = epsilons; // this is the interaction between O-H in the water molecule, we scale rmin to get the bond length - otherwise things will blow up!
					
					sigma3 = sigmas*sigmas*sigmas;
					sigma6 = sigma3*sigma3;//sigmas*sigmas*sigmas*sigmas*sigmas*sigmas;
					c6 = 4*epsilons*sigma6;
					c12 = c6*sigma6; // 4*epsilons*sigma6*sigma6; // write c12 = c6*sigma6
					//c6               = vdwparam[tj];  
                	//c12              = vdwparam[tj+1];

					//morse_potential = 463*(1-exp(-19.302077462917023*(r-0.1)))*(1-exp(-19.302077462917023*(r-0.1)));
					//morse_force = -2*463*19.302077462917023*(1-exp(-19.302077462917023*(r-0.1)))*(exp(-19.302077462917023*(r-0.1)))*rinv11;
					 
					//printf("Morse with r: %lf, morse force: %lf \n", r, morse_force);
					}
				

				}

					else {

						c6               = vdwparam[tj];  
                		c12              = vdwparam[tj+1];

					}


				}
				}

				//else {

				//c6               = 0*vdwparam[tj];  
               // c12              = 0*vdwparam[tj+1];

				//	}



				if (lj4_factor > 10)  {

				//printf("lj4_factor: %lf\n", lj4_factor);
	
				if (ATOM_Z[type[jnr]] == 11111 ||  ATOM_Z[type[ii]] == 11111)  {


					if (charge[ii] > 0.5 && charge[jnr] > 0.5) {
						
						c6               = 0;   // dont change parametsr for c6/c12
        		   	 	c12              = 0;
						morse_force		 = 0;

					}
					else {

					LJ_factor 		 = 0.5*(((ATOM_Z[type[jnr]] - charge[jnr])/ATOM_Z[type[jnr]]) + ((ATOM_Z[type[ii]] - charge[ii])/ATOM_Z[type[ii]]));
					c6               = LJ_factor*vdwparam[tj];   // dont change parametsr for c6/c12
        		    c12              = LJ_factor*vdwparam[tj+1];


	//						RMIN = 0.8*pow((2*c12)/(c6), 1.0/6.0);
	//				 if (isnan(RMIN)) { // check if the variable is NaN
					 // printf(" RMIN NaN.. c6 = %lf, c12 = %lf \n", c6, c12);
	//				morse_force = 0;
		//		}

		//		else {
					//printf("RMIN = %lf, beta = %lf  \n", RMIN, beta);
					//morse_potential = 463*(1-exp(-beta*(r-RMIN)))*(1-exp(-beta*(r-RMIN)));


		//			if (r < RMIN) {
		//				LJ_factor = 0.5*(((ATOM_Z[type[jnr]] - charge[jnr])/ATOM_Z[type[jnr]]) + ((ATOM_Z[type[ii]] - charge[ii])/ATOM_Z[type[ii]]));
		//				beta =  LJ_factor*20.16562;
		//				morse_force = -2*463*beta*(1-exp(-beta*(r-RMIN)))*(exp(-beta*(r-RMIN)))*rinv11;

		//			}
		//			else {
		//				morse_force = 0;

		//			}

		///			}
		
						}	

					//c6               = vdwparam[tj];   // dont change parametsr for c6/c12
        		    //c12              = vdwparam[tj+1];
				

				}
				else {
					
					LJ_factor = 0.5*(((ATOM_Z[type[jnr]] - charge[jnr])/ATOM_Z[type[jnr]]) + ((ATOM_Z[type[ii]] - charge[ii])/ATOM_Z[type[ii]]));
					
					lj_c6_factor = LJ_factor*LJ_factor*LJ_factor*LJ_factor*LJ_factor*LJ_factor;
					lj_c12_factor = lj_c6_factor*lj_c6_factor;

					//LJ_factor_epsilon = sqrt(scaleEpsilon(ATOM_Z[type[jnr]], charge[jnr])*scaleEpsilon(ATOM_Z[type[ii]], charge[ii]));
					//LJ_factor_sigma = 0.5*(scaleRmin(ATOM_Z[type[jnr]], charge[jnr]) +  scaleRmin(ATOM_Z[type[ii]], charge[ii]));

					//lj_c6_factor = LJ_factor_sigma*LJ_factor_sigma*LJ_factor_sigma*LJ_factor_sigma*LJ_factor_sigma*LJ_factor_sigma;
					//lj_c12_factor = lj_c6_factor*lj_c6_factor;

					if ((ATOM_Z[type[jnr]] - charge[jnr])< 0 || (ATOM_Z[type[ii]] - charge[ii])< 0 )  {

					printf("LJ_factor_sigma = %lf, LJ_factor_epsilon=%lf, ATOMZ[jnr] = %i, charge = %lf, ATOM_Z[ii] = %i, charge = %lf, typeii= %i, typejnr = %i\n",LJ_factor_epsilon, LJ_factor_sigma,  LJ_factor, ATOM_Z[type[jnr]], charge[jnr], ATOM_Z[type[ii]], charge[ii], type[ii],type[jnr]);
						exit(0);
						}


					//printf("LJ_factor = %lf, ATOMZ[jnr] = %i, charge = %lf, ATOM_Z[ii] = %i, charge = %lf, typeii= %i, typejnr = %i\n", LJ_factor, ATOM_Z[type[jnr]], charge[jnr], ATOM_Z[type[ii]], charge[ii], type[ii],type[jnr]);
					if (LJ_factor<0 || LJ_factor > 1 || (ATOM_Z[type[jnr]] - charge[jnr])< 0 || (ATOM_Z[type[ii]] - charge[ii])< 0 )  {

					printf("LJ_factor = %lf, ATOMZ[jnr] = %i, charge = %lf, ATOM_Z[ii] = %i, charge = %lf, typeii= %i, typejnr = %i\n", LJ_factor, ATOM_Z[type[jnr]], charge[jnr], ATOM_Z[type[ii]], charge[ii], type[ii],type[jnr]);
						exit(0);
						}
					//printf("LJ_factor = %lf, ATOMZ[jnr] = %i, charge = %lf, ATOM_Z[ii] = %i, charge = %lf, typeii= %i, typejnr = %i\n", LJ_factor, ATOM_Z[type[jnr]], charge[jnr], ATOM_Z[type[ii]], charge[ii], type[ii],type[jnr]);

					c6               = LJ_factor*vdwparam[tj];   // (lj_c6_factor*vdwparam[tj]);   // dont change parametsr for c6/c12
		            c12              = LJ_factor*vdwparam[tj+1];//  (lj_c12_factor*vdwparam[tj+1]);

					
			//		RMIN = 0.8*pow((2*c12)/(c6), 1.0/6.0);
			//		 if (isnan(RMIN)) { // check if the variable is NaN
			//		 // printf(" RMIN NaN.. c6 = %lf, c12 = %lf \n", c6, c12);
			//		morse_force = 0;
			//	}

			//	else {
					//printf("RMIN = %lf, beta = %lf  \n", RMIN, beta);
					//morse_potential = 463*(1-exp(-beta*(r-RMIN)))*(1-exp(-beta*(r-RMIN)));
			//		if (r < RMIN) {
			//			beta =  LJ_factor*20.16562;
			//			morse_force = -2*463*beta*(1-exp(-beta*(r-RMIN)))*(exp(-beta*(r-RMIN)))*rinv11;

			//		}
			//		else {
			//			morse_force = 0;

			//		}

			//		}



			    }


				rinvsix          = rinvsq*rinvsq*rinvsq;
                Vvdw6            = c6*rinvsix;     
                Vvdw12           = c12*rinvsix*rinvsix;
                //Vvdwtot          = Vvdwtot+(Vvdw12-Vvdw6)*exp(-r_times_debye_inverse); // sum LJ potential


                  if (ATOM_Z[type[jnr]] == 1 ||  ATOM_Z[type[ii]] == 1)  {

        	    c6               = vdwparam[tj];   // dont change parametsr for c6/c12
	            c12              = vdwparam[tj+1]; //

			  //  RMIN = pow((2*c12)/(c6), 1.0/6.0);

			  //  if (isnan(RMIN)) { // check if the variable is NaN
			  //		printf(" RMIN NaN.. c6 = %lf, c12 = %lf \n", c6, c12);
			//		morse_force = 0;
             //       exit(0);
			//	}   

                     RMIN = 0.1;
                    if (r<RMIN) {


                        beta = 0.7*1.88584194e+01;
                        vdW_force_check = -2*463*beta*(1-exp(-beta*(r-RMIN)))*(exp(-beta*(r-RMIN)))*rinv11;
                        Vvdwtot          = Vvdwtot+vdW_force_check; // sum LJ potential

                    }
                    else {

                        vdW_force_check = 0;
                        Vvdwtot          = Vvdwtot+vdW_force_check; // sum LJ potential


                    }

                }

                else {
                    Vvdwtot          = Vvdwtot+(Vvdw12-Vvdw6)*exp(-r_times_debye_inverse); // sum LJ potential
			        vdW_force_check = -1*(c12*((-1*(rinvsix*rinvsix))*debye_inverse-12*rinvsix*rinvsix*rinv11)+c6*(rinvsix*debye_inverse+ 6*rinvsix*rinv11))*exp(-r_times_debye_inverse)*rinv11;
                }


				if (Vvdwtot > 1e+10) { 
					printf("Vdwtot = %lf, r=%lf, debye= %lf, rtimesdebyeinverse= %lf, c6 = %e, c12 = %e, atomZ[jnr]= %i , atomZ[ii]= %i, chargejnr = %lf, chargeii = %lf \n", Vvdwtot, r, debye_length, r_times_debye_inverse, c6, c12, ATOM_Z[type[jnr]], ATOM_Z[type[ii]], charge[jnr], charge[ii]);
					exit(0);
			
				}


				}


				else {
				
				 // printf("LJ_factor14 = %lf \n", lj4_factor);

				    
					LJ_factor = lj4_factor;
				    lj_c6_factor = LJ_factor*LJ_factor*LJ_factor*LJ_factor*LJ_factor*LJ_factor;
					lj_c12_factor = lj_c6_factor*lj_c6_factor;



	
					///if (LJ_factor<0 || LJ_factor > 1 || (ATOM_Z[type[jnr]] - charge[jnr])< 0 || (ATOM_Z[type[ii]] - charge[ii])< 0 )  {

					///printf("LJ44_factor = %lf, ATOMZ[jnr] = %i, charge = %lf, ATOM_Z[ii] = %i, charge = %lf, typeii= %i, typejnr = %i\n", LJ_factor, ATOM_Z[type[jnr]], charge[jnr], ATOM_Z[type[ii]], charge[ii], type[ii],type[jnr]);
					//	exit(0);
						//}
			
					c6               = LJ_factor*(vdwparam[tj]);   // LJ_factor*(lj_c6_factor*vdwparam[tj]);   // dont change parametsr for c6/c12
		            c12              = LJ_factor*(vdwparam[tj+1]); // LJ_factor*(lj_c12_factor*vdwparam[tj+1]); //

		//	    RMIN = 0.8*pow((2*c12)/(c6), 1.0/6.0);
		//			 if (isnan(RMIN)) { // check if the variable is NaN
		//			 // printf(" RMIN NaN.. c6 = %lf, c12 = %lf \n", c6, c12);
		//			morse_force = 0;
		//		}

		//		else {
					//printf("RMIN = %lf, beta = %lf  \n", RMIN, beta);
					//morse_potential = 463*(1-exp(-beta*(r-RMIN)))*(1-exp(-beta*(r-RMIN)));


		//			if (r < RMIN) {
		//				beta =  LJ_factor*20.16562;
		//				morse_force = -2*463*beta*(1-exp(-beta*(r-RMIN)))*(exp(-beta*(r-RMIN)))*rinv11;

		//			}
		//			else {
		//				morse_force = 0;

		//			}

		//			}


				rinvsix          = rinvsq*rinvsq*rinvsq;
                Vvdw6            = c6*rinvsix;     
                Vvdw12           = c12*rinvsix*rinvsix;
               // Vvdwtot          = Vvdwtot+(Vvdw12-Vvdw6)*exp(-r_times_debye_inverse); // sum LJ potential


                 if (ATOM_Z[type[jnr]] == 1 ||  ATOM_Z[type[ii]] == 1)  {

        	    c6               = vdwparam[tj];   // dont change parametsr for c6/c12
	            c12              = vdwparam[tj+1]; //

			  //  RMIN = pow((2*c12)/(c6), 1.0/6.0);

			  //  if (isnan(RMIN)) { // check if the variable is NaN
			  //		printf(" RMIN NaN.. c6 = %lf, c12 = %lf \n", c6, c12);
			//		morse_force = 0;
             //       exit(0);
			//	}   
                    RMIN = 0.1;

                    if (r<RMIN) {
                        beta = 0.7*1.88584194e+01;
                        vdW_force_check = -2*463*beta*(1-exp(-beta*(r-RMIN)))*(exp(-beta*(r-RMIN)))*rinv11;
                        Vvdwtot          = Vvdwtot+vdW_force_check; // sum LJ potential
                    }

                    else { 

                        vdW_force_check = 0 ;
                        Vvdwtot          = Vvdwtot+vdW_force_check; // sum LJ potential


                    }



                }

                else {
                    Vvdwtot          = Vvdwtot+(Vvdw12-Vvdw6)*exp(-r_times_debye_inverse); // sum LJ potential
			        vdW_force_check = -1*(c12*((-1*(rinvsix*rinvsix))*debye_inverse-12*rinvsix*rinvsix*rinv11)+c6*(rinvsix*debye_inverse+ 6*rinvsix*rinv11))*exp(-r_times_debye_inverse)*rinv11;
                }




				if (Vvdwtot > 1e+10) { 
									printf("Vdwtot_lj4 = %lf, r=%lf, debye= %lf, rtimesdebyeinverse= %lf, c6 = %e, c12 = %e, atomZ[jnr]= %i , atomZ[ii]= %i, chargejnr = %lf, chargeii = %lf \n", Vvdwtot, r, debye_length, r_times_debye_inverse, c6, c12, ATOM_Z[type[jnr]], ATOM_Z[type[ii]], charge[jnr], charge[ii]);
									exit(0);
							
								}


					//c6               = LJ_factor*vdwparam[tj];// LJ_factor*(lj_c6_factor*vdwparam[tj]);   // dont change parametsr for c6/c12
		            //c12              = LJ_factor*vdwparam[tj+1];// LJ_factor*(lj_c12_factor*vdwparam[tj+1]);


				}


				//c6               = vdwparam[tj];// LJ_factor*(lj_c6_factor*vdwparam[tj]);   // dont change parametsr for c6/c12
	            //c12              = vdwparam[tj+1];//
				
				//rinvsix          = rinvsq*rinvsq*rinvsq;
                //Vvdw6            = c6*rinvsix;     
               // Vvdw12           = c12*rinvsix*rinvsix;
                //Vvdwtot          = Vvdwtot+(Vvdw12-Vvdw6)*exp(-r_times_debye_inverse); // sum LJ potential

			//	if (Vvdwtot > 1e+05) { 
		//			printf("Vdwtot = %lf, r=%lf, debye= %lf, rtimesdebyeinverse= %lf, c6 = %e, c12 = %e, atomZ[jnr]= %i , atomZ[ii]= %i, chargejnr = %lf, chargeii = %lf \n", Vvdwtot, r, debye_length, r_times_debye_inverse, c6, c12, ATOM_Z[type[jnr]], ATOM_Z[type[ii]], charge[jnr], charge[ii]);
	//				exit(0);
			
		//		}



				// check if nan

				if (isnan(Vvdwtot)) { // check if the variable is NaN
					printf("The variable is NaN: c6=%e, c12=%e, r= %lf, r*timesdebyeinv= %lf, atomZ[jnr]= %i , atomZ[ii]= %i \n", c6, c12, r, r_times_debye_inverse,ATOM_Z[type[jnr]], ATOM_Z[type[ii]]);
					printf("The variable is NaN: c6=%e, c12=%e, sigma= %lf, epsilon= %lf\n", c6, c12, sigmas, epsilons);
					exit(0);
				}
				//if (isnan(morse_potential)) { // check if the variable is NaN
				///	printf("Morse is NaN. \n");
				//	exit(0);
				//}

				  if (isnan(vdW_force_check)) { // check if the variable is NaN
					printf("Screened LJ force is NaN..\n");
					exit(0);
				}


                /* Lennard-Jones interaction */
				//if (c6 > 0 || c12 > 0) {

                //rinvsix          = rinvsq*rinvsq*rinvsq;
                //Vvdw6            = c6*rinvsix;     
                //Vvdw12           = c12*rinvsix*rinvsix;
                //Vvdwtot          = Vvdwtot+Vvdw12-Vvdw6;

                //fscal            = (vcoul+12.0*Vvdw12-6.0*Vvdw6)*rinvsq;
               // fscal            = (vcoul+12.0*Vvdw12-6.0*Vvdw6)*rinvsq;
				//fscal 			 = (vcoul*(r*inverse_debye_length + 1)+12.0*Vvdw12-6.0*Vvdw6)*rinvsq;
				

				if (hybrid_potential==0) { 


				fscal = vcoul*(r_times_debye_inverse+1)*rinvsq-1*(c12*((-1*(rinvsix*rinvsix))*debye_inverse-12*rinvsix*rinvsix*rinv11)+c6*(rinvsix*debye_inverse+ 6*rinvsix*rinv11))*exp(-r_times_debye_inverse)*rinv11; //USE THIS IF YOU WANT BOTH COULOMB AND LJ TO BE SHIELDED!! 
			
				//fscal = vcoul*(r_times_debye_inverse+1)*rinvsq + (12.0*Vvdw12-6.0*Vvdw6)*rinvsq; // regular Vdw

				//fscal = vcoul*(r_times_debye_inverse+1)*rinvsq - vdW_force_check;
		
				//fscal = vcoul*(r_times_debye_inverse+1)*rinvsq + (c12*(rinvsix*rinvsix*debye_inverse + 12*rinvsix*rinvsix*rinv11) + c6*(-rinvsix*debye_inverse-6*rinvsix*rinv11))*exp(-r_times_debye_inverse)*rinv11;

				//fscal = vcoul*(r_times_debye_inverse+1)*rinvsq + ((-exp(-r_times_debye_inverse)*(c6*rsq11*rsq11*rsq11*(6*debye_length+r)+c12*(-12*debye_length-r)))*debye_inverse*rinvsix*rinvsix)*rinvsq; // is this faster, less divisions?

				//}
				//else {
     
                //Vvdwtot          = 0;
                //fscal            = (vcoul+12.0*Vvdw12-6.0*Vvdw6)*rinvsq;
                //fscal            = (vcoul)*rinvsq;
				//fscal  			 = vcoul*(r_times_debye_inverse + 1)*rinvsq;
			
				//}

				}
				else {
				//rprime = 0;
				if ( r <= rprime)  {
					// mostly ion sphere part
					//printf(" ion sphere part\n");

					if (charge[ii] < 0.5 ||  charge[jnr] <0.5) { 
						//fscal = -1*(c12*((-1*(rinvsix*rinvsix))*debye_inverse-12*rinvsix*rinvsix*rinv11)+c6*(rinvsix*debye_inverse+ 6*rinvsix*rinv11))*exp(-r_times_debye_inverse)*rinv11;
						fscal = vdW_force_check;
				
					}   
					else {

					fscal =  (qq*(-c0*rinvsq) - ((qq*r)/(rs*rs*rs)))*rinv11 + vdW_force_check; //-1*(c12*((-1*(rinvsix*rinvsix))*debye_inverse-12*rinvsix*rinvsix*rinv11)+c6*(rinvsix*debye_inverse+ 6*rinvsix*rinv11))*exp(-r_times_debye_inverse)*rinv11;      //vcoul*(r_times_debye_inverse+1)*rinvsq

					//fscal =  (qq*(-c0*rinvsq) - ((qq*r)/(rs*rs*rs)))*rinv11 + vdW_force_check;      //vcoul*(r_times_debye_inverse+1)*rinvsq

					}
				}
				else {
					// mostly debye part
					//printf(" debye part\n");
					if (charge[ii] < 0.5 ||  charge[jnr] <0.5) { 

					//fscal = -1*(c12*((-1*(rinvsix*rinvsix))*debye_inverse-12*rinvsix*rinvsix*rinv11)+c6*(rinvsix*debye_inverse+ 6*rinvsix*rinv11))*exp(-r_times_debye_inverse)*rinv11;

					fscal = vdW_force_check;
		
					}   

					else {

					//fscal = qq*(-(c3*exp(-r_times_debye_inverse)*(r_times_debye_inverse+1))*rinvsq)*rinv11 -1*(c12*((-1*(rinvsix*rinvsix))*debye_inverse-12*rinvsix*rinvsix*rinv11)+c6*(rinvsix*debye_inverse+ 6*rinvsix*rinv11))*exp(-r_times_debye_inverse)*rinv11;
					fscal = qq*(-(c3*exp(-r_times_debye_inverse)*(r_times_debye_inverse+1))*rinvsq)*rinv11 + vdW_force_check;


					//fscal = qq*((1*exp(-r_times_debye_inverse)*(r_times_debye_inverse+1))*rinvsq)*rinv11 -1*(c12*((-1*(rinvsix*rinvsix))*debye_inverse-12*rinvsix*rinvsix*rinv11)+c6*(rinvsix*debye_inverse+ 6*rinvsix*rinv11))*exp(-r_times_debye_inverse)*rinv11;
					}
		

				}





				//-1*(c12*((-1*(rinvsix*rinvsix))*debye_inverse-12*rinvsix*rinvsix*rinv11)+c6*(rinvsix*debye_inverse+ 6*rinvsix*rinv11))*exp(-r_times_debye_inverse)*rinv11; //USE THIS IF YOU WANT BOTH COULOMB AND LJ TO BE SHIELDED!



				}


				// !! Note: for the forces, we multiply with an extra 1/r because of the direction of the force !! //

			
                /* Calculate temporary vectorial force */
                tx               = fscal*dx11;     
                ty               = fscal*dy11;     
                tz               = fscal*dz11;     

                /* Increment i atom force */
                fix1             = fix1 + tx;      
                fiy1             = fiy1 + ty;      
                fiz1             = fiz1 + tz;      

                /* Decrement j atom force */
                faction[j3+0]    = faction[j3+0] - tx;
                faction[j3+1]    = faction[j3+1] - ty;
                faction[j3+2]    = faction[j3+2] - tz;

                /* Inner loop uses 38 flops/iteration */
            }
            

            /* Add i forces to mem and shifted force list */
            faction[ii3+0]   = faction[ii3+0] + fix1;
            faction[ii3+1]   = faction[ii3+1] + fiy1;
            faction[ii3+2]   = faction[ii3+2] + fiz1;
            fshift[is3]      = fshift[is3]+fix1;
            fshift[is3+1]    = fshift[is3+1]+fiy1;
            fshift[is3+2]    = fshift[is3+2]+fiz1;

            /* Add potential energies to the group for this list */
            ggid             = gid[n];         
            Vc[ggid]         = Vc[ggid] + vctot;
            Vvdw[ggid]       = Vvdw[ggid] + Vvdwtot;

            /* Increment number of inner iterations */
            ninner           = ninner + nj1 - nj0;

            /* Outer loop uses 12 flops/iteration */
        }
        

        /* Increment number of outer iterations */
        nouter           = nouter + nn1 - nn0;
    }
    while (nn1<nri);
    

    /* Write outer/inner iteration count to pointers */
    *outeriter       = nouter;         
    *inneriter       = ninner;         
}





/*
 * Gromacs nonbonded kernel nb_kernel110nf
 * Coulomb interaction:     Normal Coulomb
 * VdW interaction:         Lennard-Jones
 * water optimization:      No
 * Calculate forces:        no
 */
void nb_kernel110nf(
                    int *           p_nri,
                    int *           iinr,
                    int *           jindex,
                    int *           jjnr,
                    int *           shift,
                    real *          shiftvec,
                    real *          fshift,
                    int *           gid,
                    real *          pos,
                    real *          faction,
                    real *          charge,
                    real *          p_facel,
                    real *          p_krf,
                    real *          p_crf,
                    real *          Vc,
                    int *           type,
                    int *           p_ntype,
                    real *          vdwparam,
                    real *          Vvdw,
                    real *          p_tabscale,
                    real *          VFtab,
                    real *          invsqrta,
                    real *          dvda,
                    real *          p_gbtabscale,
                    real *          GBtab,
                    int *           p_nthreads,
                    int *           count,
                    void *          mtx,
                    int *           outeriter,
                    int *           inneriter,
                    real *          work)
{
    int           nri,ntype,nthreads;
    real          facel,krf,crf,tabscale,gbtabscale;
    int           n,ii,is3,ii3,k,nj0,nj1,jnr,j3,ggid;
    int           nn0,nn1,nouter,ninner;
    real          shX,shY,shZ;
    real          rinvsq;
    real          iq;
    real          qq,vcoul,vctot;
    int           nti;
    int           tj;
    real          rinvsix;
    real          Vvdw6,Vvdwtot;
    real          Vvdw12;
    real          ix1,iy1,iz1;
    real          jx1,jy1,jz1;
    real          dx11,dy11,dz11,rsq11,rinv11;
    real          c6,c12;

    nri              = *p_nri;         
    ntype            = *p_ntype;       
    nthreads         = *p_nthreads;    
    facel            = *p_facel;       
    krf              = *p_krf;         
    crf              = *p_crf;         
    tabscale         = *p_tabscale;    

    /* Reset outer and inner iteration counters */
    nouter           = 0;              
    ninner           = 0;              

    /* Loop over thread workunits */
    
    do
    {
#ifdef GMX_THREAD_SHM_FDECOMP
        tMPI_Thread_mutex_lock((tMPI_Thread_mutex_t *)mtx);
        nn0              = *count;         
		
        /* Take successively smaller chunks (at least 10 lists) */
        nn1              = nn0+(nri-nn0)/(2*nthreads)+10;
        *count           = nn1;            
        tMPI_Thread_mutex_unlock((tMPI_Thread_mutex_t *)mtx);
        if(nn1>nri) nn1=nri;
#else
	    nn0 = 0;
		nn1 = nri;
#endif
        /* Start outer loop over neighborlists */
        
        for(n=nn0; (n<nn1); n++)
        {

            /* Load shift vector for this list */
            is3              = 3*shift[n];     
            shX              = shiftvec[is3];  
            shY              = shiftvec[is3+1];
            shZ              = shiftvec[is3+2];

            /* Load limits for loop over neighbors */
            nj0              = jindex[n];      
            nj1              = jindex[n+1];    

            /* Get outer coordinate index */
            ii               = iinr[n];        
            ii3              = 3*ii;           

            /* Load i atom data, add shift vector */
            ix1              = shX + pos[ii3+0];
            iy1              = shY + pos[ii3+1];
            iz1              = shZ + pos[ii3+2];

            /* Load parameters for i atom */
            iq               = facel*charge[ii];
            nti              = 2*ntype*type[ii];

            /* Zero the potential energy for this list */
            vctot            = 0;              
            Vvdwtot          = 0;              

            /* Clear i atom forces */
            
            for(k=nj0; (k<nj1); k++)
            {

                /* Get j neighbor index, and coordinate index */
                jnr              = jjnr[k];        
                j3               = 3*jnr;          

                /* load j atom coordinates */
                jx1              = pos[j3+0];      
                jy1              = pos[j3+1];      
                jz1              = pos[j3+2];      

                /* Calculate distance */
                dx11             = ix1 - jx1;      
                dy11             = iy1 - jy1;      
                dz11             = iz1 - jz1;      
                rsq11            = dx11*dx11+dy11*dy11+dz11*dz11;

                /* Calculate 1/r and 1/r2 */
                rinv11           = gmx_invsqrt(rsq11);

                /* Load parameters for j atom */
                qq               = iq*charge[jnr]; 
                tj               = nti+2*type[jnr];
                c6               = vdwparam[tj];   
                c12              = vdwparam[tj+1]; 
                rinvsq           = rinv11*rinv11;  

                /* Coulomb interaction */
                vcoul            = qq*rinv11;      
                vctot            = vctot+vcoul;    

                /* Lennard-Jones interaction */
                rinvsix          = rinvsq*rinvsq*rinvsq;
                Vvdw6            = c6*rinvsix;     
                Vvdw12           = c12*rinvsix*rinvsix;
                Vvdwtot          = Vvdwtot+Vvdw12-Vvdw6;

                /* Inner loop uses 24 flops/iteration */
            }
            

            /* Add i forces to mem and shifted force list */

            /* Add potential energies to the group for this list */
            ggid             = gid[n];         
            Vc[ggid]         = Vc[ggid] + vctot;
            Vvdw[ggid]       = Vvdw[ggid] + Vvdwtot;

            /* Increment number of inner iterations */
            ninner           = ninner + nj1 - nj0;

            /* Outer loop uses 6 flops/iteration */
        }
        

        /* Increment number of outer iterations */
        nouter           = nouter + nn1 - nn0;
    }
    while (nn1<nri);
    

    /* Write outer/inner iteration count to pointers */
    *outeriter       = nouter;         
    *inneriter       = ninner;         
}


