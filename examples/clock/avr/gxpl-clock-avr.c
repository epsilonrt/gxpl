/**
 * @brief  Simple xPL clock device that sends a time update out (AVR)
 *
 * Copyright (c) 2004, Gerald R. Duprey Jr.
 * Copyright (c) 2015-2016, epsilonRT                
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdlib.h>
#include <gxPL.h>
#include "version-git.h"

/* constants ================================================================ */
#define CLOCK_VERSION         VERSION_SHORT
#define CLOCK_UPDATE_INTERVAL 30
#define REPORT_OWN_MESSAGE    true
#define POLL_RATE_MS          1000

/* constants ================================================================ */
/* =============================================================================
 * AVR has no command line or terminal, constants below allow to setup the
 * io layer and the serial port terminal, if NLOG is defined in the makefile,
 * no terminal is used.
 * =============================================================================
 */
#define AVR_IOLAYER_NAME      "xbeezb"
#define AVR_IOLAYER_PORT      "tty1"

#define AVR_TERMINAL_PORT     "tty0"
#define AVR_TERMINAL_BAUDRATE 500000
#define AVR_TERMINAL_FLOW     SERIAL_FLOW_NONE
#include <gxPL/stdio.h>

/* private variables ======================================================== */
static gxPLApplication * app;
static gxPLDevice * device;
static gxPLMessage * message;

/* private functions ======================================================== */
static void prvSendTick (void);
static void prvMessageListener (gxPLDevice * device, gxPLMessage * msg, void * udata) ;
static void prvCloseAll (void);

/* main ===================================================================== */
int
main (int argc, char * argv[]) {
  int ret;
  gxPLSetting * setting;

  gxPLStdIoOpen();
  setting = gxPLSettingNew (AVR_IOLAYER_PORT, AVR_IOLAYER_NAME, gxPLConnectViaHub);
  assert (setting);
  setting->log = LOG_DEBUG;
  
  gxPLPrintf ("xPL Clock\nPress any key to start...\n");
  gxPLWait();

  // opens the xPL network
  app = gxPLAppOpen (setting);
  if (app == NULL) {

    PERROR ("Unable to start xPL");
    gxPLExit (EXIT_FAILURE);
  }

  // Initialize clock device
  // Create  a device for us
  device = gxPLAppAddDevice (app, "epsirt", "clock", NULL);
  assert (device);

  ret = gxPLDeviceVersionSet (device, CLOCK_VERSION);
  assert (ret == 0);

  ret = gxPLDeviceReportOwnMessagesSet (device, REPORT_OWN_MESSAGE);
  assert (ret == 0);

  // Add a responder for time setting
  ret = gxPLDeviceListenerAdd (device, prvMessageListener, gxPLMessageAny,
                               NULL, NULL, NULL);
  assert (ret == 0);

  // Create a message to send
  message = gxPLDeviceMessageNew (device, gxPLMessageStatus);
  assert (message);

  // Setting up the message
  ret = gxPLMessageBroadcastSet (message, true);
  assert (ret == 0);
  ret = gxPLMessageSchemaSet (message, "clock", "update");
  assert (ret == 0);

  gxPLPrintf ("Starting xPL Clock\n");
  gxPLPrintf ("Press Ctrl+C to abort ...\n");

  // Enable the device
  ret = gxPLDeviceEnable (device, true);
  assert (ret == 0);

  // Main Loop of Clock Action
  for (;;) {

    ret = gxPLAppPoll (app, POLL_RATE_MS);
    assert (ret == 0);

    // Process clock tick update checking
    prvSendTick();

    if (gxPLIsInterrupted()) {

      prvCloseAll();
      gxPLExit (EXIT_SUCCESS);
    }
  }
}

// -----------------------------------------------------------------------------
static void
prvSendTick (void) {

  if (gxPLDeviceIsHubConfirmed (device) == true) {
    static unsigned long last;
    unsigned long now = gxPLTime();

    // Skip unless the delay has passed
    if ( (last == 0) || ( (now - last) >= CLOCK_UPDATE_INTERVAL)) {
      int ret;
      char * strtime = gxPLDateTimeStr (now, NULL);

      // Install the value and send the message
      ret = gxPLMessagePairSet (message, "time", strtime);
      assert (ret >= 0);

      // Broadcast the message
      ret = gxPLDeviceMessageSend (device, message);
      assert (ret >= 0);

      PINFO ("broadcast clock.update #%d", ret);

      // And reset when we last sent the clock update
      last = now;
    }
  }
}

// -----------------------------------------------------------------------------
// message handler
static void
prvMessageListener (gxPLDevice * device, gxPLMessage * msg, void * udata) {

  PINFO ("received %s msg from %s-%s.%s with %s.%s schema",
         gxPLMessageTypeToString (gxPLMessageTypeGet (msg)),
         gxPLMessageSourceVendorIdGet (msg),
         gxPLMessageSourceDeviceIdGet (msg),
         gxPLMessageSourceInstanceIdGet (msg),
         gxPLMessageSchemaClassGet (msg),
         gxPLMessageSchemaTypeGet (msg));
}

// -----------------------------------------------------------------------------
static void
prvCloseAll (void) {
  int ret;

  // Sends heartbeat end messages to all devices
  ret = gxPLAppDisableAllDevices (app);
  assert (ret == 0);

  gxPLPrintf ("\nPress any key to close...\n");
  gxPLWait();

  ret = gxPLAppClose (app);
  assert (ret == 0);

  gxPLMessageDelete (message);

  gxPLPrintf ("\neverything was closed.\nHave a nice day !\n");
}

/* ========================================================================== */
