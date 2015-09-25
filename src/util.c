/**
 * @file util.c
 * Misc support for gxPLib
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <gxPL/util.h>

/* macros =================================================================== */
/* constants ================================================================ */
/* structures =============================================================== */
/* types ==================================================================== */
/* private variables ======================================================== */
/* private functions ======================================================== */
/* public variables ========================================================= */
/* public api functions ===================================================== */

// -----------------------------------------------------------------------------
// name=value\0
gxPLPair *
gxPLPairFromString (char * str) {
  if (str) {
    char * name;

    char * value = str;

    name = strsep (&value, "=");
    if (value) {
      gxPLPair * p = malloc (sizeof (gxPLPair));
      assert (p);
      p->name = malloc (strlen (name) + 1);
      p->value = malloc (strlen (value) + 1);
      strcpy (p->name, name);
      strcpy (p->value, value);
      return p;
    }
  }
  return NULL;
}

// -----------------------------------------------------------------------------
int
gxPLStrCpy (char * dst, const char * src) {
  int c;
  int count = 0;
  char * p = dst;

  while ( (c = *src) != 0) {

    if (isascii (c)) {

      if ( (!isalnum (c)) && (c != '-')) {

        // c is not a letter, number or hyphen/dash
        errno = EINVAL;
        return -1;
      }

      if (isupper (c)) {

        // convert to lowercase
        *p = tolower (c);
      }
      else {

        // raw copy
        *p = c;
      }

      p++;
      src++;
      count++;
    }
    else {

      // c is not a ASCII character
      errno = EINVAL;
      return -1;
    }
  }
  return count;
}

// -----------------------------------------------------------------------------
int
gxPLIdSet (gxPLId * id, const char * vendor_id, const char * device_id, const char * instance_id) {

  if (gxPLIdVendorIdSet (id, vendor_id) == 0) {

    if (gxPLIdDeviceIdSet (id, device_id) == 0) {

      return gxPLIdInstanceIdSet (id, instance_id);
    }
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLIdVendorIdSet (gxPLId * id, const char * vendor_id) {
  if ((id == NULL) || (vendor_id == NULL)) {
    errno = EFAULT;
    return -1;
  }

  if (strlen (vendor_id) > GXPL_VENDORID_MAX) {
    errno = EINVAL;
    return -1;
  }
  return (gxPLStrCpy (id->vendor, vendor_id) > 0) ? 0 : -1;
}

// -----------------------------------------------------------------------------
int
gxPLIdDeviceIdSet (gxPLId * id, const char * device_id) {
  if ((id == NULL) || (device_id == NULL)) {
    errno = EFAULT;
    return -1;
  }

  if (strlen (device_id) > GXPL_DEVICEID_MAX) {
    errno = EINVAL;
    return -1;
  }
  return (gxPLStrCpy (id->device, device_id) > 0) ? 0 : -1;
}

// -----------------------------------------------------------------------------
int
gxPLIdInstanceIdSet (gxPLId * id, const char * instance_id) {
  if ((id == NULL) || (instance_id == NULL)) {
    errno = EFAULT;
    return -1;
  }

  if (strlen (instance_id) > GXPL_INSTANCEID_MAX) {
    errno = EINVAL;
    return -1;
  }
  return (gxPLStrCpy (id->instance, instance_id) > 0) ? 0 : -1;
}

// -----------------------------------------------------------------------------
int
gxPLIdCopy (gxPLId * dst, const gxPLId * src) {

  if (gxPLIdVendorIdSet (dst, src->vendor) == 0) {

    if (gxPLIdDeviceIdSet (dst, src->device) == 0) {

      return gxPLIdInstanceIdSet (dst, src->instance);
    }
  }
  return -1;
}


// -----------------------------------------------------------------------------
int
gxPLIdCmp (const gxPLId * n1, const gxPLId * n2) {
  int ret = strcmp (n1->vendor, n2->vendor);
  if (ret == 0) {
    ret = strcmp (n1->device, n2->device);
    if (ret == 0) {
      ret = strcmp (n1->instance, n2->instance);
    }
  }
  return ret;
}

// -----------------------------------------------------------------------------
// vendor-device.instance\0
int
gxPLIdFromString (gxPLId * id, char * str) {
  char *p, *n;

  n = str;
  p = strsep (&n, "-");
  if (n) {
    if (gxPLIdVendorIdSet (id, p) == 0) {

      p = strsep (&n, ".");
      if (n) {

        if (gxPLIdDeviceIdSet (id, p) == 0) {

          return gxPLIdInstanceIdSet (id, n);
        }
      }
    }

  }
  errno = EINVAL;
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLSchemaCmp (const gxPLSchema * s1, const gxPLSchema * s2) {
  int ret = strcmp (s1->class, s2->class);
  if (ret == 0) {
    ret = strcmp (s1->type, s2->type);
  }
  return ret;

}
/* ========================================================================== */
