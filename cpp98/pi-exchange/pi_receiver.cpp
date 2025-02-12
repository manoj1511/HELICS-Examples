/*

Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
static char help[] = " PI RECEIVER: Simple program to demonstrate the usage of HELICS C Interface.\n\
            This example creates a value federate subscribing to the publication \n\
            registered by PI SENDER.\n\n";

#include <stdio.h>
#include <helics/cpp98/ValueFederate.hpp>
#include <helics/cpp98/helics.hpp>

int main(int /*argc*/,char ** /*argv*/)
{
  std::string    fedinitstring="--federates=1";
  double         deltat=0.01;
  helicscpp::Input sub;


  std::string helicsversion = helicscpp::getHelicsVersionString();

  printf("PI RECEIVER: Helics version = %s\n",helicsversion.c_str());
  printf("%s",help);

  /* Create Federate Info object that describes the federate properties
   * Set federate name and core type from string
   */
  helicscpp::FederateInfo fi( "zmq");

  /* Federate init string */
  fi.setCoreInitString(fedinitstring);

  /* Set the message interval (timedelta) for federate. Note that
     HELICS minimum message time interval is 1 ns and by default
     it uses a time delta of 1 second. What is provided to the
     setTimedelta routine is a multiplier for the default timedelta.
  */
  /* Set one second message interval */
  fi.setProperty(helics_property_time_delta, deltat);
  fi.setProperty(helics_property_int_log_level, helics_log_level_warning);

  /* Create value federate */
  helicscpp::ValueFederate* vfed = new helicscpp::ValueFederate("TestB Federate", fi);
  printf("PI RECEIVER: Value federate created\n");

  /* Subscribe to PI SENDER's publication */
  sub = vfed->registerSubscription("testA","double");
  printf("PI RECEIVER: Subscription registered\n");

  /* Enter initialization state */
  vfed->enterInitializingMode(); // can throw helicscpp::InvalidStateTransition exception
  printf("PI RECEIVER: Entered initialization state\n");

  /* Enter execution state */
  vfed->enterExecutingMode(); // can throw helicscpp::InvalidStateTransition exception
  printf("PI RECEIVER: Entered execution state\n");

  helics_time currenttime=0.0;
  double        value = 0.0;

  while(currenttime < 0.20) {
    currenttime = vfed->requestTime(currenttime);

    int isupdated = sub.isUpdated();
    if(isupdated) {
      /* NOTE: The value sent by sender at time t is received by receiver at time t+deltat */
      value = sub.getDouble();
      printf("PI RECEIVER: Received value = %4.3f at time %3.2f from PI SENDER\n",value,currenttime);
    }
  }
  vfed->finalize();
  printf("PI RECEIVER: Federate finalized\n");
  // Destructor must be called before close library
  delete vfed;
  helicsCloseLibrary();
  return(0);
}

