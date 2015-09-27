/**
 * @file
 * @brief  Simple xPL clock device that sends a time update out
 *
 * Copyright (c) 2004, Gerald R. Duprey Jr.
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <assert.h>
#include <gxPL.h>
#include "version-git.h"

/* constants ================================================================ */
#define CLOCK_VERSION         VERSION_SHORT
#define CLOCK_UPDATE_INTERVAL 10
#define REPORT_OWN_MESSAGE    true

/* private variables ======================================================== */
static gxPL * net;
static gxPLDevice * clk;
static gxPLMessage * message;

/* private functions ======================================================== */
static void prvSignalHandler (int s);
static void prvSendTick (void);
static void prvMessageListener (gxPLDevice * device, const gxPLMessage * msg, void * udata) ;

/* main ===================================================================== */
int
main (int argc, char * argv[]) {
  int ret;
  gxPLConfig * config;

  config = gxPLConfigNewFromCommandArgs (argc, argv, gxPLConnectViaHub);
  assert (config);

  // opens the xPL network
  net = gxPLOpen (config);
  if (net == NULL) {

    fprintf (stderr, "Unable to start xPL");
    exit (EXIT_FAILURE);
  }

  // Initialize clock service
  // Create  a service for us
  clk = gxPLDeviceAdd (net, "epsirt", "clock", NULL);
  assert (clk);

  ret = gxPLDeviceVersionSet (clk, CLOCK_VERSION);
  assert (ret == 0);
  
  ret = gxPLDeviceReportOwnMessagesSet (clk, REPORT_OWN_MESSAGE);
  assert (ret == 0);

  // Add a responder for time setting
  ret = gxPLDeviceListenerAdd (clk, prvMessageListener, gxPLMessageAny,
                               "clock", NULL, NULL);
  assert (ret == 0);

  // Create a message to send
  message = gxPLDeviceMessageNew (clk, gxPLMessageStatus);
  assert (message);
  // Setting up the message
  ret = gxPLMessageBroadcastSet (message, true);
  assert (ret == 0);
  ret = gxPLMessageSchemaSet (message, "clock", "update");
  assert (ret == 0);

  // Install signal traps for proper shutdown
  signal (SIGTERM, prvSignalHandler);
  signal (SIGINT, prvSignalHandler);

  // Enable the service
  ret = gxPLDeviceEnabledSet (clk, true);
  assert (ret == 0);

  // Main Loop of Clock Action
  for (;;) {

    // Let XPL run for a while, returning after it hasn't seen any
    // activity in 100ms or so
    ret = gxPLPoll (net, 100);

    // Process clock tick update checking
    prvSendTick();
  }
}

// -----------------------------------------------------------------------------
static void
prvSendTick (void) {
  static time_t last;
  time_t now = time (NULL);

  // Skip unless the delay has passed
  if ( (last == 0) || ( (now - last) >= CLOCK_UPDATE_INTERVAL)) {
    struct tm * t;
    char str[24];
    
    // Format the date/time
    t = localtime (&now);
    strftime (str, 24, "%Y%m%d%H%M%S", t);

    // Install the value and send the message
    gxPLMessagePairValueSet (message, "time", str);

    // Broadcast the message
    gxPLDeviceMessageSend (clk, message);

    // And reset when we last sent the clock update
    last = now;
  }
}

// -----------------------------------------------------------------------------
// message handler
static void
prvMessageListener (gxPLDevice * device, const gxPLMessage * msg, void * udata) {

  printf ("Received a Clock Message from %s-%s.%s of type %d for %s.%s\n",
          gxPLMessageSourceVendorIdGet (msg),
          gxPLMessageSourceDeviceIdGet (msg),
          gxPLMessageSourceInstanceIdGet (msg),
          gxPLMessageTypeGet (msg),
          gxPLMessageSchemaClassGet (msg),
          gxPLMessageSchemaTypeGet (msg));
}

// -----------------------------------------------------------------------------
// signal handler
static void
prvSignalHandler (int s) {
  int ret;

  // all devices will be deactivated and destroyed before closing
  ret = gxPLClose (net);
  assert (ret == 0);
  gxPLMessageDelete (message);

  printf ("\neverything was closed.\nHave a nice day !\n");
  exit (EXIT_SUCCESS);
}

/* ========================================================================== */
