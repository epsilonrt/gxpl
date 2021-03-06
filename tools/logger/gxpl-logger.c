/**
 * @file
 * Simple program to monitor for any message and any device changes and print them
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), epsilonRT                
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <getopt.h>

#include <gxPL.h>
#include "version-git.h"

/* constants ================================================================ */
#define LOGGER_VERSION VERSION_SHORT
#define LOG_FILE_CFG_NAME "filename"
#define LOG_APPEND_CFG_NAME "append"
#define LOGGER_VENDOR "epsirt"
#define LOGGER_DEVICE "logger"
#define DEFAULT_CONFIG_FILE "gxpl-logger.xpl"

/* private variables ======================================================== */
static gxPLApplication * app;
static gxPLDevice * device;

/* configurable items ======================================================= */
static FILE * logfile = NULL;
static const char * log_filename = "";
static bool append_log = false;

/* private functions ======================================================== */
static void prvSignalHandler (int s);
static void prvSetConfig (gxPLDevice * device);
static void prvConfigChanged (gxPLDevice * device, void * udata);
static void prvPrintMessage (gxPLApplication * app, gxPLMessage * message, void * udata);
static void prvPrintUsage (void);

/* main ===================================================================== */
int
main (int argc, char * argv[]) {
  int c, ret;
  gxPLSetting * setting;
  static const char short_options[] = "h" GXPL_GETOPT;
  static struct option long_options[] = {
    {"help",     no_argument,        NULL, 'h' },
    {NULL, 0, NULL, 0} /* End of array need by getopt_long do not delete it*/
  };

  setting = gxPLSettingFromCommandArgs (argc, argv, gxPLConnectViaHub);
  assert (setting);
  
  do  {

    c = getopt_long (argc, argv, short_options, long_options, NULL);

    switch (c) {

      case 'h':
        prvPrintUsage();
        free (setting);
        exit (EXIT_SUCCESS);
        break;

      default:
        break;
    }
  }
  while (c != -1);

  // opens the xPL network
  app = gxPLAppOpen (setting);
  if (app == NULL) {

    fprintf (stderr, "Unable to start xPL");
    exit (EXIT_FAILURE);
  }

  // Add a listener for all xPL messages
  ret = gxPLMessageListenerAdd (app, prvPrintMessage, NULL);
  assert (ret == 0);

  // Create a configurable device and set our application version
  device = gxPLAppAddConfigurableDevice (app, LOGGER_VENDOR, LOGGER_DEVICE,
                                         gxPLConfigPath (DEFAULT_CONFIG_FILE));
  assert (device);

  ret = gxPLDeviceVersionSet (device, LOGGER_VERSION);
  assert (ret == 0);

  if (gxPLDeviceIsConfigured (device) == false) {

    // Define a configurable item and give it a default
    ret = gxPLDeviceConfigItemAdd (device, LOG_FILE_CFG_NAME, gxPLConfigReconf, 1);
    assert (ret == 0);
    ret = gxPLDeviceConfigValueSet (device, LOG_FILE_CFG_NAME, "stderr");
    assert (ret == 0);

    ret = gxPLDeviceConfigItemAdd (device, LOG_APPEND_CFG_NAME, gxPLConfigReconf, 1);
    assert (ret == 0);
    ret = gxPLDeviceConfigValueSet (device, LOG_APPEND_CFG_NAME, "false");
    assert (ret == 0);
  }

  // Parse the device configurables into a form this program
  // can use (whether we read a config or not)
  prvSetConfig (device);

  // Add a device change listener we'll use to pick up changes
  ret = gxPLDeviceConfigListenerAdd (device, prvConfigChanged, NULL);
  assert (ret == 0);

  // Enable the device
  gxPLDeviceEnable (device, true);

  // Install signal traps for proper shutdown
  signal (SIGTERM, prvSignalHandler);
  signal (SIGINT, prvSignalHandler);

  // Enable the service
  ret = gxPLDeviceEnable (device, true);
  assert (ret == 0);

  for (;;) {
    // Let XPL run for a while, returning after it hasn't seen any
    // activity in 100ms or so
    ret = gxPLAppPoll (app, 100);
    assert (ret == 0);
  }
  return 0;
}

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
// It's best to put the logic for reading the device configuration
// and parsing it into your code in a seperate function so it can
// be used by your configChangedHandler and your startup code that
// will want to parse the same data after a config file is loaded
static void
prvSetConfig (gxPLDevice * device) {
  const char * str;
  FILE * new_file = NULL;

  // Get append status
  if ( (str = gxPLDeviceConfigValueGet (device, LOG_APPEND_CFG_NAME)) != NULL) {

    append_log = strcasecmp (str, "true") == 0;
  }

  // Get log file name and see if it's changed
  if ( (str = gxPLDeviceConfigValueGet (device, LOG_FILE_CFG_NAME)) != NULL) {

    // Log file changed and not blank -- try to open the new one
    if ( (strlen (str) != 0) && (strcmp (log_filename, str) != 0)) {

      // Attempt to open new log file
      if (strcasecmp (str, "stderr") == 0) {

        new_file = stderr;
      }
      else if (strcasecmp (str, "stdout") == 0) {

        new_file = stdout;
      }
      else {

        new_file = fopen (str, (append_log ? "a" : "w"));
      }

      // If there is no new log file, it's bad -- ignore this and move on
      if (new_file == NULL) {

        return;
      }

      // Close out old log file (unless it was the console
      if ( (logfile != NULL) && (logfile != stderr) && (logfile != stdout)) {

        fflush (logfile);
        fclose (logfile);
      }

      // Install new file name and handle
      log_filename = str;
      logfile = new_file;
    }
  }
}


// --------------------------------------------------------------------------
// Print info on incoming messages
static void
prvPrintMessage (gxPLApplication * app, gxPLMessage * message, void * udata) {

  fprintf (logfile, " %s [xpl - message] type = %s",
           gxPLDateTimeStr (gxPLTime(), "%y/%m/%d %H:%M:%S"),
           gxPLMessageTypeToString (gxPLMessageTypeGet (message)));


  // Print hop count, if interesting
  if (gxPLMessageHopGet (message) != 1) {

    fprintf (logfile, ", hops = %d", gxPLMessageHopGet (message));
  }

  // Source Info
  fprintf (logfile, ", source = %s-%s.%s, target = ",
           gxPLMessageSourceVendorIdGet (message),
           gxPLMessageSourceDeviceIdGet (message),
           gxPLMessageSourceInstanceIdGet (message));

  // Handle various target types
  if (gxPLMessageIsBroadcast (message)) {

    fprintf (logfile, "*");
  }
  else {

    fprintf (logfile, " %s-%s.%s",
             gxPLMessageTargetVendorIdGet (message),
             gxPLMessageTargetDeviceIdGet (message),
             gxPLMessageTargetInstanceIdGet (message));
  }

  // Echo Schema Info
  fprintf (logfile, ", class = %s, type = %s",
           gxPLMessageSchemaClassGet (message),
           gxPLMessageSchemaTypeGet (message));
  fprintf (logfile, "\n");
}

// --------------------------------------------------------------------------
//  Handle a change to the device device configuration */
static void
prvConfigChanged (gxPLDevice * device, void * udata) {

  // Read setting items for device and install */
  prvSetConfig (device);
}

// -----------------------------------------------------------------------------
// signal handler
static void
prvSignalHandler (int s) {
  int ret;

  if ( (logfile != NULL) && (logfile != stderr) && (logfile != stdout)) {

    fflush (logfile);
    fclose (logfile);
  }

  // all devices will be deactivated and destroyed before closing
  ret = gxPLAppClose (app);
  assert (ret == 0);

  printf ("\neverything was closed.\nHave a nice day !\n");
  exit (EXIT_SUCCESS);
}

// -----------------------------------------------------------------------------
// Print usage info
static void
prvPrintUsage (void) {
  printf ("%s - xPL Message Logger\n", __progname);
  printf ("Copyright (c) 2015-2016 epsilonRT                \n\n");
  printf ("Usage: %s [-i interface] [-n network] [-W timeout] [options]\n", __progname);
  printf ("  -i interface - use interface named interface (i.e. eth0)"
          " as network interface\n");
  printf ("  -n network   - use hardware abstraction layer to access the network"
          " (i.e. udp, xbeezb... default: udp)\n");
  printf ("  -W timeout   - set the timeout at the opening of the io layer\n");
  printf ("  -B baudrate  - set serial baudrate (if iolayer use serial port)\n");
  printf ("  -r           - performed iolayer reset (if supported)\n");
  printf ("  -d           - enable debugging, it can be doubled or tripled to"
          " increase the level of debug. \n");
  printf ("  -h           - print this message\n");
}

/* ========================================================================== */
