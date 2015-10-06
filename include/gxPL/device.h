/**
 * @file include/gxPL/.h
 * High level interface to manage xPL devices (public header)
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_SERVICE_HEADER_
#define _GXPL_SERVICE_HEADER_

#include <sysio/vector.h>
#include <gxPL/defs.h>
__BEGIN_C_DECLS
/* ========================================================================== */

/**
 * @defgroup xPLDevices Devices
 * High level interface for manage xPL devices
 * @{
 */

/* types ==================================================================== */

/**
 * @brief Listener for a device
 */
typedef void (* gxPLDeviceListener) (gxPLDevice *, gxPLMessage *, void *);

/* structures =============================================================== */
/**
 * @brief Describe a element of configuration
 */
typedef struct  _gxPLDeviceConfigItem {

  char * name;
  gxPLConfigurableType type;
  uint8_t values_max;
  xVector values; /**< vector of strings */
} gxPLDeviceConfigItem;


/* internal public functions ================================================ */

/**
 * @brief Create a message for the device
 *
 * The message can be modified using the functions of the message module
 * before being sent with gxPLDeviceMessageSend(). A message to send can not
 * be gxPLMessageAny type. The message should be released with gxPLMessageDelete
 * after use.
 *
 * @param device pointer on the device
 * @param type the type of message
 * @return  the message, NULL if an error occurs
 */
gxPLMessage * gxPLDeviceMessageNew (gxPLDevice * device, gxPLMessageType type);

/**
 * @brief Send a message out from this device
 *
 * The source of the message should be the device but no check is performed.
 *
 * @param device pointer on the device
 * @param message
 * @return number of bytes send, -1 if error occurs
 */
int gxPLDeviceMessageSend (gxPLDevice * device, gxPLMessage * message);

/**
 * @brief Add a listener for the device
 *
 * This function allows the user to install a listener that will be called for
 * each message received for the service. \n
 * Only messages matching type, schema_class and schema_type are forwarded to
 * the listener.
 *
 * @param device pointer on the device
 * @param listener the function listening device messages
 * @param type type of message to be processed, gxPLMessageAny to manage any
 * @param schema_class schema class to process, NULL to manage everything
 * @param schema_type schema type to process, NULL to manage everything
 * @param udata pointer to the user data that will be passed to the listener,
 * NULL if not used.
 * @return 0, -1 if error occurs
 */
int gxPLDeviceListenerAdd (gxPLDevice * device,
                           gxPLDeviceListener listener,
                           gxPLMessageType type,
                           char * schema_class, char * schema_type,
                           void * udata);

/**
 * @brief Remove a device listener
 * @param device pointer on the device
 * @param listener the listener
 * @return 0, -1 if error occurs
 */
int gxPLDeviceListenerRemove (gxPLDevice * device,
                              gxPLDeviceListener listener);

/**
 * @brief Gets the parent gxPLApplication object
 *
 * @param device pointer on the device
 * @return pointer to an gxPLApplication object that is the parent of the device, NULL if error occurs
 */
gxPLApplication * gxPLDeviceParentGet (gxPLDevice * device);

/**
 * @brief Gets the identifier
 * @param device pointer on the device
 * @return the identifier, NULL if error occurs
 */
const gxPLId * gxPLDeviceId (const gxPLDevice * device);

/**
 * @brief Indicates whether the device is enabled.
 * @param device pointer on the device
 * @return true if enabled, false if not, -1 if an error occurs
 */
int gxPLDeviceIsEnabled (const gxPLDevice * device);

/**
 * @brief Gets the heartbeat interval
 *
 * @param device pointer on the device
 * @return the interval in seconds, -1
 */
int gxPLDeviceHeartbeatInterval (const gxPLDevice * device);

/**
 * @brief Gets the time of the last heartbeat sent
 *
 * The time is given in a unit dependent on the host system, most of the time
 * is given in seconds. On a unix system with a real time clock that value is
 * the number of seconds since the first second of January 1, 1970.
 *
 * @param device pointer on the device
 * @return the last time, -1 if error occurs
 */
long gxPLDeviceHeartbeatLast (const gxPLDevice * device);

/**
 * @brief Gets the version string
 *
 * This information is transmitted with the heartbeat.
 * @param device pointer on the device
 * @return the last time, NULL if error occurs
 */
const char * gxPLDeviceVersion (const gxPLDevice * device);

/**
 * @brief Indicates whether the device will respond to broadcast messages.
 * @param device pointer on the device
 * @return true if respond, false if not, -1 if an error occurs
 */
int gxPLDeviceIsRespondToBroadcast (const gxPLDevice * device);

/**
 * @brief Indicates whether the device will transmit its own messages to the listeners
 * @param device pointer on the device
 * @return true, false, -1 if an error occurs
 */
int gxPLDeviceIsReportOwnMessages (const gxPLDevice * device);

/**
 * @brief Indicates whether the device has detected a hub.
 * @param device pointer on the device
 * @return true, false, -1 if an error occurs
 */
int gxPLDeviceIsHubConfirmed (const gxPLDevice * device);

/**
 * @brief Indicates whether the device is configurable
 * @param device pointer on the device
 * @return true, false, -1 if an error occurs
 */
int gxPLDeviceIsConfigurale (const gxPLDevice * device);

/**
 * @brief Indicates whether the device is configured
 * @param device pointer on the device
 * @return true, false, -1 if an error occurs
 */
int gxPLDeviceIsConfigured (const gxPLDevice * device);


/**
 * @brief Sets the identifier
 * @param device pointer on the device
 * @param id pointer to identifier
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceIdSet (gxPLDevice * device,  const gxPLId * id);

/**
 * @brief Sets the vendor identifier
 * @param device pointer on the device
 * @param vendor_id pointer to the vendor id
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceVendorIdSet (gxPLDevice * device, const char * vendor_id);

/**
 * @brief Sets the device identifier
 * @param device pointer on the device
 * @param device_id pointer to the device id
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceDeviceIdSet (gxPLDevice * device, const char * device_id);

/**
 * @brief Sets the instance identifier
 * @param device pointer on the device
 * @param instance_id pointer to the instance id
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceInstanceIdSet (gxPLDevice * device, const char * instance_id);

/**
 * @brief Sets the version
 * @param device pointer on the device
 * @param version
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceVersionSet (gxPLDevice * device, const char * version);

/**
 * @brief Enabled or not a device
 * @param device pointer on the device
 * @param enabled true for eanbled
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceEnabledSet (gxPLDevice * device, bool enabled);

/**
 * @brief Sets the heartbeat interval
 * @param device pointer on the device
 * @param interval interval in seconds
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceHeartbeatIntervalSet (gxPLDevice * device, int interval);

/**
 * @brief Enable the response to broadcast messages
 * @param device pointer on the device
 * @param respond true for respond
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceRespondToBroadcastSet (gxPLDevice * device, bool respond);

/**
 * @brief Process own messages
 * @param device pointer on the device
 * @param isreportownmsg true or flase
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceReportOwnMessagesSet (gxPLDevice * device, bool isreportownmsg);

/**
 * @defgroup xPLDeviceGroup Groups
 * @{
 */

/**
 * @brief Adds a group to the device
 *
 * @param device pointer on the device
 * @param group_name new group
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceGroupAdd (gxPLDevice * device, const char * group_name);

/**
 * @brief Adds a group to the device form a string
 *
 * @param device pointer on the device
 * @param str string xpl-group.name where name is the group name
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceGroupAddFromString (gxPLDevice * device, const char * str);
/**
 * @brief Indicates whether the device has groups
 *
 * @param device pointer on the device
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceGroupHave (const gxPLDevice * device);

/**
 * @brief Erases all groups
 *
 * @param device pointer on the device
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceGroupClearAll (gxPLDevice * device);

/**
 * @brief Number of groups
 *
 * @param device pointer on the device
 * @return the value, -1 if an error occurs
 */
int gxPLDeviceGroupCount (const gxPLDevice * device);

/**
 * @brief Gets a group
 *
 * @param device pointer on the device
 * @param index index of the group to read
 * @return the group, NULL if error occurs
 */
const char * gxPLDeviceGroupGet (const gxPLDevice * device, int index);

/**
 * @}
 */

/**
 * @defgroup xPLDeviceFilter Filters
 * @{
 */

/**
 * @brief Adds a filter to the device
 *
 * @param device pointer on the device
 * @param filter_name new filter
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceFilterAdd (gxPLDevice * device, gxPLMessageType type,
                         const gxPLId * source, const gxPLSchema * schema);

/**
 * @brief Adds a filter to the device from a string
 * 
 * the format of the string is as follows:
 * @code
 * filter = [msgtype].[vendor].[device].[instance].[class].[type]
 * @endcode
 * for example:
 * - xpl-cmnd.wmute.k400.bedroom.drapes.basic 
 * - xpl-cmnd.wmute.k400.bedroom.drapes.*
 * - xpl-cmnd.wmute.k400.bedroom.*.*
 * - xpl-cmnd.wmute.k400.*.drapes.basic
 * - .
 * 
 * @param device pointer on the device
 * @param filter_name new filter as a string, will be modified by the function 
 * unusable after the call.
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceFilterAddFromStr (gxPLDevice * device, char * filter);

/**
 * @brief filter to string
 * @param filter the filter
 * @return the string
 */
const char * gxPLDeviceFilterToString (const gxPLFilter * filter);

/**
 * @brief Indicates whether the device has filters
 *
 * @param device pointer on the device
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceFilterHave (const gxPLDevice * device);

/**
 * @brief Erases all filters
 *
 * @param device pointer on the device
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceFilterClearAll (gxPLDevice * device);

/**
 * @brief Number of filters
 *
 * @param device pointer on the device
 * @return the value, -1 if an error occurs
 */
int gxPLDeviceFilterCount (const gxPLDevice * device);

/**
 * @brief Gets a filter
 *
 * @param device pointer on the device
 * @param index index of the filter to read
 * @return the filter, NULL if error occurs
 */
const char * gxPLDeviceFilterGet (const gxPLDevice * device, int index);

/**
 * @}
 */

/**
 * @defgroup xPLDeviceConfig Configurable devices
 * @{
 */


/* types ==================================================================== */

/**
 * @brief Changes to a devices configuration
 */
typedef void (* gxPLDeviceConfigListener) (gxPLDevice *, void *);

/* structures =============================================================== */

/* internal public functions ================================================ */

/**
 * @brief Return the installed config file, if any
 * @param device
 * @return
 */
int gxPLDeviceConfigFilenameSet (gxPLDevice * device, const char * filename);

/**
 * @brief Return the installed config file, if any
 * @param device
 * @return
 */
const char * gxPLDeviceConfigFilenameGet (const gxPLDevice * device);

/**
 * @brief Add a device config change listener
 * @param device
 * @param listener
 * @param udata
 */
int gxPLDeviceConfigListenerAdd (gxPLDevice * device,
                                 gxPLDeviceConfigListener listener,
                                 void * udata) ;

/**
 * @brief Remove a config change listener
 * @param device
 * @param listener
 * @return
 */
int gxPLDeviceConfigListenerRemove (gxPLDevice * device,
                                    gxPLDeviceConfigListener listener);

/**
 * @brief Add a new configurable
 *
 * If the item is added, true is returned.  If the item already exists,
 * false is returned and it's not added or altered
 * @param device
 * @param name
 * @param type
 * @param max_values
 * @return
 */
int gxPLDeviceConfigItemAdd (gxPLDevice * device, const char * name,
                             gxPLConfigurableType type, int max_values);

/**
 * @brief Remove a configurable.
 * @param device
 * @param name
 * @return Return true if item found and removed, false if not found
 */
int gxPLDeviceConfigItemRemove (gxPLDevice * device, const char * name);

/**
 * @brief Remove all configurables
 * @param device
 */
int gxPLDeviceConfigItemRemoveAll (gxPLDevice * device);

/**
 * @brief Clear all configurable values out
 *
 * The configurable definitions remain intact
 * @param device
 */
int gxPLDeviceConfigItemClearAll (gxPLDevice * device);

/**
 * @brief Search for a configurable and return it (or NULL)
 * @param device
 * @param name
 * @return
 */
gxPLDeviceConfigItem * gxPLDeviceConfigItemFind (const gxPLDevice * device,
    const char * name);

/**
 * @brief Return the number of values for a given configurable
 * @param device
 * @param name
 * @return
 */
int gxPLDeviceConfigValueCount (const gxPLDevice * device, const char * name);

/**
 * @brief Add a device item value
 *
 * If there are already values this is added to it, up to the limit defined for
 * the configurable.  If the item is "full", then the value is discarded.
 * @param device
 * @param name
 * @param value
 * @return
 */
int gxPLDeviceConfigValueAdd (gxPLDevice * device,
                              const char * name, const char * value);


/**
 * @brief Simple form to set first/only value in an item
 *
 * @param device
 * @param name
 * @param value
 */
int gxPLDeviceConfigValueSet (gxPLDevice * device,
                              const char * name, const char * value);

/**
 * @brief Set a item value at a given index.
 *
 * If that index is above the actual number of values, the value is appeneded
 * (i.e. may not be the same index as passed)
 * @param device
 * @param name
 * @param index
 * @param value
 */
int gxPLDeviceConfigValueSetAt (gxPLDevice * device, const char * name,
                                int index, const char * value);

/**
 * @brief Clear values for a given configurable
 * @param device
 * @param name
 */
int gxPLDeviceConfigValueClearAll (gxPLDevice * device, const char * name);

/**
 * @brief Return the value of the first/only index for an item
 * @param device
 * @param name
 * @return
 */
const char * gxPLDeviceConfigValueGet (gxPLDevice * device, const char * name);

/**
 * @brief Return the value at the given index.
 * If the value is NULL of the index is out of range, NULL is returned
 * @param device
 * @param name
 * @param index
 * @return
 */
const char * gxPLDeviceConfigValueGetAt (gxPLDevice * device, const char * name, int index);

/**
 * @}
 */

/**
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_SERVICE_HEADER_ defined */
