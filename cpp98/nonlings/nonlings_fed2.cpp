/*

Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include <stdio.h>
#include <math.h>
#include <helics/cpp98/ValueFederate.hpp>
#include <helics/cpp98/helics.hpp> // helicsVersionString

int main(int /*argc*/,char ** /*argv*/)
{
  std::string    fedinitstring="--federates=1";
  double         deltat=0.01;
  helicscpp::Input sub;
  helicscpp::Publication  pub;


  std::string helicsversion = helicscpp::getHelicsVersionString();

  printf(" Helics version = %s\n",helicsversion.c_str());

  /* Create Federate Info object that describes the federate properties
   * Set federate name and core type from string
   */
  helicscpp::FederateInfo fi ( "zmq");

  /* Federate init string */
  fi.setCoreInitString (fedinitstring);

  fi.setProperty(helics_property_time_delta, deltat);
  fi.setProperty(helics_property_int_log_level, helics_log_level_warning);
  fi.setProperty(helics_property_int_max_iterations, 100);

  /* Create value federate */
  helicscpp::ValueFederate* vfed = new helicscpp::ValueFederate ("TestB Federate", fi);
  printf(" Value federate created\n");

  sub = vfed->registerSubscription("testA");
  printf(" Subscription registered\n");

  /* Register the publication */
  pub = vfed->registerGlobalTypePublication("testB","double");
  printf(" Publication registered\n");


  /* Enter initialization state */
  vfed->enterInitializingMode(); // can throw helicscpp::InvalidStateTransition exception
  printf(" Entered initialization state\n");
  double y = 1.0, /*xprv = 100,*/yprv=100;

  pub.publish(y);
  fflush(NULL);
  /* Enter execution state */
  vfed->enterExecutingMode(); // can throw helicscpp::InvalidStateTransition exception
  printf(" Entered execution state\n");

  fflush(NULL);
  helics_time currenttime=0.0;
  helicscpp::helics_iteration_time currenttimeiter;
  currenttimeiter.status = helics_iteration_result_iterating;

 // int           isupdated;
  double tol=1E-8;
  int helics_iter = 0;
  while (currenttimeiter.status== helics_iteration_result_iterating)
  {

//    xprv = x;
    double x = sub.getDouble();
    ++helics_iter;
    int    newt_conv = 0, max_iter=10,iter=0;

    /* Solve the equation using Newton */
    while(!newt_conv && iter < max_iter) {
      /* Function value */
      double f2 = x*x + 4*y*y - 4;

      if(fabs(f2) < tol) {
	newt_conv = 1;
	break;
      }
      iter++;

      /* Jacobian */
      double J2 = 8*y;

      y = y - f2/J2;
    }
    printf("Fed2 iteration %d y=%f, x=%f\n",helics_iter,y,x);


    if ((fabs(y-yprv)>tol)||(helics_iter<5))
    {
      pub.publish(y);
      printf("Fed2: publishing y\n");
    }
    fflush(NULL);
    currenttimeiter = vfed->requestTimeIterative(currenttime, helics_iteration_request_iterate_if_needed);
    yprv = y;
  }

  vfed->finalize();
  printf("NLIN2: Federate finalized\n");
  fflush(NULL);
  // Destructor for ValueFederate must be called before close library
  delete vfed;
  helicsCloseLibrary();
  printf("NLIN2: Library Closed\n");
  fflush(NULL);
  return(0);

}

