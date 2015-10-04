/**
 * @file gxpl.c
 * Top Layer of API (source code)
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include "config.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sysio/dlist.h>

#include <gxPL.h>
#define GXPL_INTERNALS
#include <gxPL/io.h>
#include <gxPL/util.h>
#include <gxPL/device.h>
#include "gxpl_p.h"
#include "version-git.h"

/* macros =================================================================== */
/* constants ================================================================ */
/* structures =============================================================== */
/* structures =============================================================== */
typedef struct _listener_elmt {
  gxPLMessageListener func;
  void * data;
} listener_elmt;

/* types ==================================================================== */
/* private variables ======================================================== */

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
static const void *
prvListenerKey (const void * elmt) {
  listener_elmt * p = (listener_elmt *) elmt;

  return & (p->func);
}

// -----------------------------------------------------------------------------
static int
prvListenerMatch (const void *key1, const void *key2) {

  return * ( (gxPLMessageListener *) key1) == * ( (gxPLMessageListener *) key2);
}

// -----------------------------------------------------------------------------
static const void *
prvDeviceKey (const void * elmt) {
  gxPLDevice * d = (gxPLDevice *) elmt;

  return gxPLDeviceId (d);
}

// -----------------------------------------------------------------------------
static int
prvDeviceMatch (const void *key1, const void *key2) {

  return gxPLIdCmp ( (gxPLId *) key1, (gxPLId *) key2);
}

// -----------------------------------------------------------------------------
static void
prvDeviceDelete (void * d) {

  gxPLDeviceDelete ( (gxPLDevice *) d);
}

// -----------------------------------------------------------------------------
// Public
// Stop (disable) all services, usually in preparation for shutdown, but
// that isn't the only possible reason
int gxPLDeviceDisableAll (gxPL * gxpl) {
  gxPLDevice * device;

  for (int i = 0; i < iVectorSize (&gxpl->device); i++) {

    device = pvVectorGet (&gxpl->device, i);
    if (gxPLDeviceEnabledSet (device, false) != 0) {
      return -1;
    }
  }
  return 0;
}

/* -----------------------------------------------------------------------------
 * Private
 * Check each known device for when it last sent a heart
 * beat and if it's time to send another, do it. */
static void
prvHeartbeatPoll (gxPL * gxpl) {
  gxPLDevice * device;
  long now = gxPLTime();
  long elapsed;

  for (int i = 0; i < iVectorSize (&gxpl->device); i++) {

    device = pvVectorGet (&gxpl->device, i);

    if (gxPLDeviceIsEnabled (device)) {
      int interval = gxPLDeviceHeartbeatInterval (device);
      time_t last = gxPLDeviceHeartbeatLast (device);

      // See how much time has gone by
      elapsed = now - last;

      if (gxPLDeviceIsHubConfirmed (device)) {

        if (last >= 1) {

          if (gxPLDeviceIsConfigurale (device) &&
              !gxPLDeviceIsConfigured (device)) {

            // If we are in configuration mode, we send out once a minute
            if (elapsed < DEFAULT_CONFIG_HEARTBEAT_INTERVAL) {

              continue;
            }
          }

          // For normal heartbeats, once each "hbeat_interval"
          if (elapsed < interval) {

            continue;
          }
        }
      }
      else {

        // If we are still waiting to hear from the hub,
        // then send a message every 3 seconds until we do
        if (elapsed < DEFAULT_HUB_DISCOVERY_INTERVAL) {

          continue;
        }
      }

      gxPLDeviceHeartbeatSend (device, gxPLHeartbeatHello);
    }
  }
}

// -----------------------------------------------------------------------------
// Run the passed message by each device and see who is interested
static void
prvDeviceMessageDispatcher (gxPL * gxpl, gxPLMessage * message,
                            void * udata) {
  gxPLDevice * device;

  for (int i = 0; i < iVectorSize (&gxpl->device); i++) {

    device = pvVectorGet (&gxpl->device, i);
    gxPLDeviceMessageHandler (device, message, udata);
  }
}

// -----------------------------------------------------------------------------
static void
prvEncodeLong (unsigned long value, char * str, int size) {
  int i, len, str_len = strlen (str);
  static const char alphanum[] =
    "0123456789"
    "abcdefghijklmnopqrstuvwxyz";
  const int base = sizeof (alphanum) - 1;

  // Fill with zeros
  for (i = str_len; i < size; i++) {

    str[i] = '0';
  }
  str[size] = '\0';
  len = strlen (str);

  // Handle the simple case
  if (value == 0) {
    return;
  }

  for (i = len - 1; i >= (len - str_len); i--) {

    str[i] = alphanum[value % base];
    if (value < base) {

      break;
    }
    value = value / base;
  }
}

/* internal public functions ================================================ */


/* api functions ============================================================ */
// -----------------------------------------------------------------------------
gxPLSetting *
gxPLSettingNew (const char * iface, const char * iolayer, gxPLConnectType type) {
  gxPLSetting * setting = calloc (1, sizeof (gxPLSetting));
  assert (setting);

  strcpy (setting->iface, iface);
  strcpy (setting->iolayer, iolayer);
  setting->connecttype = type;
  setting->malloc = 1;

  return setting;
}

// -----------------------------------------------------------------------------
gxPLSetting *
gxPLSettingNewFromCommandArgs (int argc, char * argv[], gxPLConnectType type) {
  gxPLSetting * setting = calloc (1, sizeof (gxPLSetting));
  assert (setting);

  gxPLParseCommonArgs (setting, argc, argv);
  if (strlen (setting->iolayer) == 0) {

    strcpy (setting->iolayer, DEFAULT_IO_LAYER);
  }
  setting->connecttype = type;
  setting->malloc = 1;

  return setting;
}

// -----------------------------------------------------------------------------
gxPL *
gxPLOpen (gxPLSetting * setting) {
  gxPL * gxpl = calloc (1, sizeof (gxPL));
  assert (gxpl);

  if (setting->debug) {

    vLogSetMask (LOG_UPTO (GXPL_LOG_DEBUG_LEVEL));
  }

  gxpl->io = gxPLIoOpen (setting);

  if (gxpl->io) {

    if (setting->malloc == 0) {

      gxpl->setting = malloc (sizeof (gxPLSetting));
      memcpy (gxpl->setting, setting, sizeof (gxPLSetting));
      gxpl->setting->malloc = 1;
    }
    else {

      gxpl->setting = setting;
    }

    if (iVectorInit (&gxpl->msg_listener, 2, NULL, free) == 0) {

      if (iVectorInitSearch (&gxpl->msg_listener, prvListenerKey,
                             prvListenerMatch) == 0) {

        if (iVectorInit (&gxpl->device, 2, NULL, prvDeviceDelete) == 0) {

          if (iVectorInitSearch (&gxpl->device, prvDeviceKey,
                                 prvDeviceMatch) == 0) {

            // everything was done, we copy the network information and returns.
            (void) gxPLIoCtl (gxpl, gxPLIoFuncGetLocalAddr, &gxpl->net_info);
            if (gxPLMessageListenerAdd (gxpl, prvDeviceMessageDispatcher, NULL) == 0) {
              
              srand (gxPLRandomSeed (gxpl));
              return gxpl;
            }
          }
        }
      }
    }
  }
  PERROR ("Unable to setup gxPL object");
  free (gxpl);
  return NULL;
}

// -----------------------------------------------------------------------------
int
gxPLClose (gxPL * gxpl) {

  if (gxpl) {
    int ret;

    // for each device, sends a goodbye heartbeat and removes all listeners,
    vVectorDestroy (&gxpl->device);
    // then releases all message listeners
    vVectorDestroy (&gxpl->msg_listener);
    // an close !
    ret = gxPLIoClose (gxpl->io);
    if (gxpl->setting->malloc) {

      free (gxpl->setting);
    }
    free (gxpl);
    return ret;
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLMessageListenerAdd (gxPL * gxpl, gxPLMessageListener listener, void * udata) {
  listener_elmt * h = malloc (sizeof (listener_elmt));
  assert (h);

  h->func = listener;
  h->data = udata;

  if (iVectorAppend (&gxpl->msg_listener, h) == 0) {
    return 0;
  }
  free (h);
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLMessageListenerRemove (gxPL * gxpl, gxPLMessageListener listener) {
  int i = iVectorFindFirstIndex (&gxpl->msg_listener, &listener);


  return iVectorRemove (&gxpl->msg_listener, i);
}

// -----------------------------------------------------------------------------
int
gxPLPoll (gxPL * gxpl, int timeout_ms) {
  int ret, size = 0;

  ret = gxPLIoCtl (gxpl, gxPLIoFuncPoll, &size, timeout_ms);

  if (ret == 0)  {

    if (size > 0) {
      char * buffer = malloc (size + 1);
      assert (buffer);

      ret = gxPLIoRecv (gxpl->io, buffer, size, NULL);
      if (ret == size) {
        static gxPLMessage * msg;

        // We receive a message, append null character to terminate the string
        buffer[size] = '\0';
        PDEBUG ("Just read %d bytes, raw buffer below >>>\n%s<<<", size, buffer);

        // TODO: Send the raw message to any raw message msg_listener ?

        msg = gxPLMessageFromString (msg, buffer);
        if (msg) {

          if (gxPLMessageIsError (msg)) {
            vLog (LOG_INFO, "Error parsing network message - ignored");
          }
          else if (gxPLMessageIsValid (msg)) {

            // Dispatch the message
            PDEBUG ("Now dispatching valid message");

            for (int i = 0; i < iVectorSize (&gxpl->msg_listener); i++) {

              listener_elmt * h = pvVectorGet (&gxpl->msg_listener, i);
              if (h->func) {

                h->func (gxpl, msg, h->data);
              }
            }
          }

          if (gxPLMessageIsValid (msg) || gxPLMessageIsError (msg)) {

            // Release the message
            gxPLMessageDelete (msg);
            msg = NULL;
            ret = 0;
          }
        }
        else {

          vLog (LOG_INFO, "Error parsing network message - ignored");
        }
      }
      free (buffer);
    }
    else {

      prvHeartbeatPoll (gxpl);
    }
  }
  else {

    vLog (LOG_INFO, "Error Polling");
  }
  return ret;
}

// -----------------------------------------------------------------------------
int
gxPLMessageSend (gxPL * gxpl, gxPLMessage * message) {
  int ret = -1;
  int count;
  char * str = gxPLMessageToString (message);

  if (str) {

    count = strlen (str);
    ret = gxPLIoSend (gxpl->io, str, count, NULL);
    if (ret < 0) {
      PERROR ("Unable to send message: [%10s...]", str);
    }
    free (str);
  }

  return ret;
}


// -----------------------------------------------------------------------------
int
gxPLMessageIsHubEcho (const gxPL * gxpl, const gxPLMessage * msg,
                      const gxPLId * my_id) {

  if ( (strcmp (gxPLMessageSchemaClassGet (msg), "hbeat") == 0) ||
       (strcmp (gxPLMessageSchemaClassGet (msg), "config") == 0)) {
    if (strcmp (gxPLMessageSchemaTypeGet (msg), "app") == 0) {
      const char * remote_ip = gxPLMessagePairGet (msg, "remote-ip");

      if (remote_ip) {
        const char * str_port = gxPLMessagePairGet (msg, "port");

        if (str_port) {
          char *endptr;

          uint16_t port = strtol (str_port, &endptr, 10);

          if (*endptr == '\0') {
            if ( (port == gxpl->net_info.port) &&
                 (strcmp (gxPLIoLocalAddrGet (gxpl), remote_ip) == 0)) {

              return true;
            }
          }
        }
      }
    }
    else if (strcmp (gxPLMessageSchemaClassGet (msg), "basic") == 0) {

      if (my_id) {
        if (gxPLIdCmp (my_id, gxPLMessageSourceIdGet (msg)) == 0) {

          return true;
        }
      }
    }
  }
  return false;
}

// -----------------------------------------------------------------------------
gxPLDevice *
gxPLDeviceAdd (gxPL * gxpl, const char * vendor_id,
               const char * device_id, const char * instance_id) {

  gxPLDevice * device = gxPLDeviceNew (gxpl, vendor_id, device_id, instance_id);
  if (device) {

    // verifies that the device does not exist
    if (pvVectorFindFirst (&gxpl->device, gxPLDeviceId (device)) == NULL) {

      // if not, add it to the list
      if (iVectorAppend (&gxpl->device, device) == 0) {

        return device;
      }
    }
  }
  // failure exit
  gxPLDeviceDelete (device);
  return NULL;
}

// -----------------------------------------------------------------------------
gxPLDevice *
gxPLDeviceConfigAdd (gxPL * gxpl, const char * vendor_id,
                     const char * device_id, const char * filename) {
  gxPLDevice * device = gxPLDeviceConfigNew (gxpl,
                        vendor_id, device_id, filename);
  if (device) {

    // verifies that the device does not exist
    if (pvVectorFindFirst (&gxpl->device, gxPLDeviceId (device)) == NULL) {

      // if not, add it to the list
      if (iVectorAppend (&gxpl->device, device) == 0) {

        return device;
      }
    }
  }
  // failure exit
  gxPLDeviceDelete (device);
  return NULL;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceRemove (gxPL * gxpl, gxPLDevice * device) {

  int index = iVectorFindFirstIndex (&gxpl->device, device);
  if (index >= 0) {
    return iVectorRemove (&gxpl->device, index);
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceCount (gxPL * gxpl) {

  return iVectorSize (&gxpl->device);
}

// -----------------------------------------------------------------------------
gxPLDevice *
gxPLDeviceAt (gxPL * gxpl, int index) {

  return (gxPLDevice *) pvVectorGet (&gxpl->device, index);
}

// -----------------------------------------------------------------------------
int
gxPLDeviceIndex (gxPL * gxpl, const gxPLDevice * device) {

  return iVectorFindFirstIndex (&gxpl->device, device);
}

// -----------------------------------------------------------------------------
int
gxPLIoCtl (gxPL * gxpl, int c, ...) {
  int ret = 0;
  va_list ap;

  va_start (ap, c);
  switch (c) {

      // put here the requests should not be transmitted to the layer below.
      // case ...

    default:
      ret = gxPLIoIoCtl (gxpl->io, c, ap);
      if ( (ret == -1) && (errno == EINVAL)) {
        PERROR ("gxPLIoCtl function not supported: %d", c);
      }
      break;
  }

  va_end (ap);
  return ret;
}

// -----------------------------------------------------------------------------
const char *
gxPLIoLocalAddrGet (const gxPL * gxpl) {
  static char * str;

  if (gxPLIoCtl ( (gxPL *) gxpl, gxPLIoFuncNetAddrToString, &gxpl->net_info, &str) == 0) {

    return str;
  }
  return "";
}

// -----------------------------------------------------------------------------
const char *
gxPLIoBcastAddrGet (const gxPL * gxpl) {
  gxPLIoAddr addr;
  static char * str;

  if (gxPLIoCtl ( (gxPL *) gxpl, gxPLIoFuncGetBcastAddr, &addr) == 0) {

    if (gxPLIoCtl ( (gxPL *) gxpl, gxPLIoFuncNetAddrToString, &addr, &str) == 0) {

      return str;
    }
  }
  return "";
}

// -----------------------------------------------------------------------------
const gxPLIoAddr *
gxPLIoInfoGet (const gxPL * gxpl) {

  return &gxpl->net_info;
}

// -----------------------------------------------------------------------------
gxPLConnectType
gxPLConnectionTypeGet (const gxPL * gxpl) {

  return gxpl->setting->connecttype;
}

// -----------------------------------------------------------------------------
const char *
gxPLIoInterfaceGet (const gxPL * gxpl) {

  return gxpl->setting->iface;
}

// -----------------------------------------------------------------------------
const char *
gxPLIoLayerGet (const gxPL * gxpl) {

  return gxpl->setting->iolayer;
}

// -----------------------------------------------------------------------------
const char *
gxPLVersion (void) {

  return VERSION_SHORT;
}

// -----------------------------------------------------------------------------
int
gxPLVersionMajor (void) {

  return VERSION_MAJOR;
}

// -----------------------------------------------------------------------------
int
gxPLVersionMinor (void) {

  return VERSION_MINOR;
}

// -----------------------------------------------------------------------------
int
gxPLVersionPatch (void) {

  return VERSION_PATCH;
}

// -----------------------------------------------------------------------------
int
gxPLVersionSha1 (void) {

  return VERSION_SHA1;
}

// -----------------------------------------------------------------------------
int
gxPLGenerateUniqueId (const gxPL * gxpl, char * s, int size) {
  int max, len = 0;

  if (gxpl->net_info.addrlen > 0) {

    for (int i = 0; (i < gxpl->net_info.addrlen) && (len < size); i++) {

      max = size - len + 1;
      len += snprintf (&s[len], max, "%02x", gxpl->net_info.addr[i]);
    }
    if (len > size) {

      len = size;
    }
  }
  if (len < size) {
    unsigned long ms;

    if (gxPLTimeMs (&ms) == 0) {

      prvEncodeLong (ms, s, size);
      gxPLTimeDelayMs (1);
    }
  }

  return strlen (s);
}

/* ========================================================================== */
