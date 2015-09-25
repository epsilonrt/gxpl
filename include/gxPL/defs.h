/**
 * @file gxPL/defs.h
 * gxPL Defs
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_DEFS_HEADER_
#define _GXPL_DEFS_HEADER_

#include <sysio/defs.h>
__BEGIN_C_DECLS
/* ========================================================================== */
#include <errno.h>

/* forward struct defined */
typedef struct _gxPL gxPL;
typedef struct _gxPLIo gxPLIo;
typedef struct _gxPLDevice gxPLDevice;
typedef struct _gxPLMessage gxPLMessage;
typedef struct _gxPLDeviceChangedListenerDef gxPLDeviceChangedListenerDef;
typedef struct _gxPLDeviceConfigurable gxPLDeviceConfigurable;

#if defined(SYSIO_OS_UNIX)
#include <limits.h>
#else
#define NAME_MAX 255
#endif

#ifndef EINVAL
#define EINVAL          22      /* Invalid argument */
#endif

/**
 * @defgroup xPLDefs Definitions
 * @{
 */

/* constants ================================================================ */

/**
 * Communications between xPL applications on a Local Area Network (LAN) use
 * UDP on port 3865
 */
#define XPL_PORT 3865

/**
 * @brief Maximum number of characters allowed for vendor ID
 */
#define GXPL_VENDORID_MAX   8

/**
 * @brief Maximum number of characters allowed for device ID
 */
#define GXPL_DEVICEID_MAX   8

/**
 * @brief Maximum number of characters allowed for instance ID
 */
#define GXPL_INSTANCEID_MAX 16

/**
 * @brief Maximum number of characters allowed for schema class
 */
#define GXPL_CLASS_MAX      8

/**
 * @brief Maximum number of characters allowed for schema type
 */
#define GXPL_TYPE_MAX       8

/**
 * @brief Maximum number of characters allowed for a name of a name/value pair
 */
#define GXPL_NAME_MAX       16

/**
 * @brief Maximum number of hop count
 */
#define GXPL_HOP_MAX   9

/**
 * @brief xPL Connection mode
 */
typedef enum {
  gxPLConnectStandAlone, /**< listen on xPL port */
  gxPLConnectViaHub,     /**< listen on a client port */
  gxPLConnectAuto
} gxPLConnectType;

/**
 * @brief xPL Configurable Type
 */
typedef enum {
  gxPLConfigOptional,
  gxPLConfigMandatory,
  gxPLConfigReconf
} gxPLConfigurableType;

/**
 * @brief Possible xPL message types
 */
typedef enum {
  gxPLMessageAny,
  gxPLMessageCommand,
  gxPLMessageStatus,
  gxPLMessageTrigger
} gxPLMessageType;


/**
 * @brief Possible xPL ioctl call
 */
typedef enum {
  gxPLIoFuncNone = 0,
  gxPLIoFuncPoll,
  gxPLIoFuncGetIface,
  gxPLIoFuncGetBcastAddr,
  gxPLIoFuncGetLocalAddr,
  gxPLIoFuncNetAddrToString,
  gxPLIoFuncError = -1
} gxPLIoFunc;

/**
 * @brief Net families
 */
typedef enum {
  gxPLNetFamilyInet     = 2, /**< family & gxPLNetFamilyInet -> true for two revisions of IP (v4 and v6) */
  gxPLNetFamilyInet4    = gxPLNetFamilyInet, 
  gxPLNetFamilyInet6    = gxPLNetFamilyInet | 1,
  gxPLNetFamilyZigbee   = 4,
  gxPLNetFamilyZigbee16 = gxPLNetFamilyZigbee,
  gxPLNetFamilyZigbee64 = gxPLNetFamilyZigbee | 1
} gxPLNetFamily;

/**
 * @brief Decoding states of a message
 */
typedef enum {
  gxPLMessageStateInit = 0,
  gxPLMessageStateHeader,
  gxPLMessageStateHeaderHop,
  gxPLMessageStateHeaderSource,
  gxPLMessageStateHeaderTarget,
  gxPLMessageStateHeaderEnd,
  gxPLMessageStateSchema,
  gxPLMessageStateBodyBegin,
  gxPLMessageStateBody,
  gxPLMessageStateBodyEnd,
  gxPLMessageStateEnd,
  gxPLMessageStateError = -1
} gxPLMessageState;

/* types ==================================================================== */

/* structures =============================================================== */
/**
 * @brief Describe a gxPL configuration
 */
typedef struct _gxPLConfig {

  char iface[NAME_MAX]; /**< interface name */
  char iolayer[NAME_MAX]; /**< io layer name */
  gxPLConnectType connecttype;
  union {
    unsigned int flag;
    struct {
      unsigned int debug: 1;  /**< debug enabled */
      unsigned int malloc: 1; /**< this configuration has been allocated on the heap and should be released. */
    };
  };
} gxPLConfig;

/**
 * @brief Describe a network address
 */
typedef struct _gxPLIoAddr {
  gxPLNetFamily family; /**< network family */
  uint8_t addrlen;  /**< number of bytes of the address */
  uint8_t addr[16]; /**< address in network order */
  int port;  /**< port in host order, -1 if not use */
  union {
    uint16_t flag;
    struct {
      uint16_t isbroadcast: 1;
    };
  };
} gxPLIoAddr;

/**
 * @brief Describe a source or destination identifier
 */
typedef struct _gxPLId {
  char vendor[GXPL_VENDORID_MAX + 1]; /**< vendor id */
  char device[GXPL_DEVICEID_MAX + 1]; /**< devide id */
  char instance[GXPL_INSTANCEID_MAX + 1]; /**< instance id */
} gxPLId;

/**
 * @brief Describe a message schema
 */
typedef struct _gxPLSchema {
  char class[GXPL_CLASS_MAX + 1];
  char type[GXPL_TYPE_MAX + 1];
} gxPLSchema;

/**
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_DEFS_HEADER_ defined */
