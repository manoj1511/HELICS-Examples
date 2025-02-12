/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

/* Example form http://mathfaculty.fullerton.edu/mathews//n2003/newtonsystem/newtonsystemproof.pdf
*/

#include <stdio.h>
#include <helics/shared_api_library/ValueFederate.h>
#include <math.h>

/* This solves the system being simulated by this simulator. It takes in the coupling variable
   x, convergence tolerance tol, and returns the state variable yout and the converged status converged
*/
void run_sim2(double x,double tol,double *yout,int *converged)
{
  double y=*yout;
  int    newt_conv = 0, max_iter=10,iter=0;

  /* Solve the equation using Newton */
  while(!newt_conv && iter < max_iter) {
      double J2;
    /* Function value */
    double f2 = x*x + 4*y*y - 4;

    /* Convergence check */
    if(fabs(f2) < tol) {
      newt_conv = 1;
      break;
    }
    iter++;

    /* Jacobian */
    J2 = 8*y;

    /* Update */
    y = y - f2/J2;
  }
  *yout = y;
  *converged = newt_conv;
}

int main()
{
  helics_federate_info fedinfo;
  const char*    helicsversion;
  const char*    fedinitstring="--federates=1";
  double         deltat=0.01;
  helics_federate vfed;
  helics_input sub;
  helics_publication  pub;
  int converged;
  int stringLength;
  char sendbuf[100],recvbuf[100];
  double y = 1.0, /*xprv = 100,*/ yprv=100;
  int my_conv=0,other_conv; /* Global and local convergence */
  helics_time currenttime=0.0;
  helics_iteration_result currenttimeiter=helics_iteration_result_iterating;
  double tol=1E-8;
  int helics_iter = 0;
  helics_error err= helicsErrorInitialize();

  helicsversion = helicsGetVersion();

  printf(" Helics version = %s\n",helicsversion);

  /* Create Federate Info object that describes the federate properties */
  fedinfo = helicsCreateFederateInfo();

  /* Set core type from string */
  helicsFederateInfoSetCoreTypeFromString(fedinfo,"zmq",NULL);

  /* Federate init string */
  helicsFederateInfoSetCoreInitString(fedinfo,fedinitstring,NULL);

  helicsFederateInfoSetTimeProperty(fedinfo, helics_property_time_period, deltat, &err);
  helicsFederateInfoSetIntegerProperty(fedinfo, helics_property_int_max_iterations, 100, &err);
  helicsFederateInfoSetIntegerProperty(fedinfo, helics_property_int_log_level, 1, &err);

  /* Create value federate */
  vfed = helicsCreateValueFederate("TestB Federate",fedinfo,NULL);
  printf(" Value federate created\n");

  sub = helicsFederateRegisterSubscription(vfed,"testA","",NULL);
  printf(" Subscription registered\n");

  /* Register the publication */
  pub = helicsFederateRegisterGlobalPublication(vfed,"testB",helics_data_type_string,"",NULL);
  printf(" Publication registered\n");

  /* Enter initialization mode */
  helicsFederateEnterInitializingMode(vfed,&err);
  if (err.error_code != helics_ok) {
      printf("Error entering Initialization\n");
      return(err.error_code);
  }
  printf(" Entered initialization mode\n");

  snprintf(sendbuf,100,"%18.16f,%d",y,my_conv);
  helicsPublicationPublishString(pub, sendbuf,&err);
  if (err.error_code != helics_ok) {
    printf("Error sending publication:%s\n",err.message);
  }
  fflush(NULL);
  /* Enter execution mode */
  helicsFederateEnterExecutingMode(vfed,&err);
  printf(" Entered execution mode\n");

  fflush(NULL);

  while (currenttimeiter==helics_iteration_result_iterating) {
      int global_conv = 0;
      double x = 0.0;
    helicsInputGetString(sub,recvbuf,100, &stringLength,NULL);
    sscanf(recvbuf,"%lf,%d",&x,&other_conv);

    /* Check for global convergence */
    global_conv = my_conv&other_conv;

    if(global_conv) {
		currenttime=helicsFederateRequestTimeIterative(vfed, currenttime, helics_iteration_request_no_iteration,&currenttimeiter,&err);
    } else {
      /* Solve the system of equations for this federate */
      run_sim2(x,tol,&y,&converged);

      ++helics_iter;
      printf("Fed2 Current time %4.3f iteration %d x=%f, y=%f\n",currenttime, helics_iter,x,y);

      if ((fabs(y-yprv)>tol)) {
	my_conv = 0;
	printf("Fed2: publishing new y\n");
      } else {
	my_conv = 1;
	printf("Fed2: converged\n");
      }

      snprintf(sendbuf,100,"%18.16f,%d",y,my_conv);
      helicsPublicationPublishString(pub, sendbuf,&err);

      fflush(NULL);
	  currenttime=helicsFederateRequestTimeIterative(vfed, currenttime, helics_iteration_request_force_iteration,&currenttimeiter,&err);
      yprv = y;
    }
  }

  helicsFederateFinalize(vfed,&err);
  printf("NLIN2: Federate finalized\n");
  fflush(NULL);
  helicsFederateFree(vfed);
  helicsCloseLibrary();
  printf("NLIN2: Library Closed\n");
  fflush(NULL);
  return(0);
}

