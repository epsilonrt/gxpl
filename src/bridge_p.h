/**
 * @file
 * gxPLBridge internal include
 * 
 * Copyright 2015 (c), epsilonRT                
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License") 
 */
#ifndef _GXPL_BRIDGE_PRIVATE_HEADER_
#define _GXPL_BRIDGE_PRIVATE_HEADER_

#include <gxPL/defs.h>

/* structures =============================================================== */

/**
 * @brief Describes a bridge client
 */
typedef struct _gxPLBridgeClient {
  
  gxPLIoAddr addr;
  gxPLId id;
  int hbeat_period_max; /**< (hbeat_interval * 2 + 60) */
  long hbeat_last;
} gxPLBridgeClient;

/**
 * @brief Describes a xPL to xPL bridge
 */
typedef struct _gxPLBridge {
  gxPLApplication * in;
  gxPLApplication * out;
  gxPLDevice * device;
  xVector clients;
  xVector allow;
  long timeout;
  uint8_t max_hop; /* only messages with a hop count less than or equal to max_hop cross the bridge */
} gxPLBridge;

/* ========================================================================== */
#endif /* _GXPL_BRIDGE_PRIVATE_HEADER_ defined */
