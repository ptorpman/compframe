/*************************************************************************
 *
 * uuid.h         Standalone implementation of DCE Universal Unique IDs
 *                Jim Doyle, Boston University, 10-04-1998
 * 
 * Derived from Public DCE 1.1 Sourcebase:
 * 
 * (c) Copyright 1989 OPEN SOFTWARE FOUNDATION, INC.
 * (c) Copyright 1989 HEWLETT-PACKARD COMPANY
 * (c) Copyright 1989 DIGITAL EQUIPMENT CORPORATION
 * To anyone who acknowledges that this file is provided "AS IS"
 * without any express or implied warranty:
 *                 permission to use, copy, modify, and distribute this
 * file for any purpose is hereby granted without fee, provided that
 * the above copyright notices and this notice appears in all source
 * code copies, and that none of the names of Open Software
 * Foundation, Inc., Hewlett-Packard Company, or Digital Equipment
 * Corporation be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  Neither Open Software Foundation, Inc., Hewlett-
 * Packard Company, nor Digital Equipment Corporation makes any
 * representations about the suitability of this software for any
 * purpose.
 * 
 *************************************************************************/ 

#include <stddef.h>

#ifndef _DCE_PROTOTYPE_
#define _DCE_PROTOTYPE_(x)         x
#endif

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE !FALSE
#endif

#ifdef __cplusplus
    extern "C" {
#endif

#define uuid_c_version (1)
#define uuid_c_version_highest (2)

  /*
   * Internal structure of a DCE UUID (128-bits)
   * Size of component types *IS IMPORTANT*
   */

/** A type */
struct __uuid_internal_t {
  unsigned int   time_low;                        /* 32 bits */
  unsigned short time_mid;                        /* 16 bits */
  unsigned short time_hi_and_version;             /* 16 bits */
  unsigned char  clock_seq_hi_and_reserved;       /* 8 bits */
  unsigned char  clock_seq_low;                   /* 8 bits */
  unsigned char  node[6];                         /* 48 bit IEEE 802 addr */
};

      /** A type */
struct __old_uuid_internal_t  {
  unsigned int time_high;                         /* 32 bits */
  unsigned short int time_low;                    /* 16 bits */
  unsigned short int reserved;                    /* 16 bits */
  unsigned char family;                           /* 8 bits */
  unsigned char host[7];                          /* 64 bit host address */
};

typedef struct __old_uuid_internal_t uuid_old_t;       
typedef struct __uuid_internal_t     uuid_t;
typedef uuid_t *                     uuid_p_t;

  /*
   * the DCE codebase uses sized types
   */

typedef unsigned int       boolean;
typedef unsigned long      error_status_t;
typedef unsigned int       unsigned32;
typedef int                signed32;
typedef unsigned int       boolean32;
typedef short int          signed16;
typedef unsigned short int unsigned16;
typedef unsigned char *    unsigned_char_p_t;
typedef unsigned char      unsigned8;
typedef unsigned char *    byte_p_t;

  /*
   * MAC address type
   */

#define IEEE_802_FILE                   "/etc/ieee_802_addr"

      /** A type */
  typedef struct dce_802_addr_s_t {
    unsigned char     eaddr[6];
  } dce_802_addr_t; 

/*
 * Max size of a uuid string: tttttttt-tttt-cccc-cccc-nnnnnnnnnnnn
 * Note: this includes the implied '\0'
 */

#define UUID_C_UUID_STRING_MAX          37

  /*
   * Error Status Codes for the UUID Library
   */

#define error_status_ok                 (0)
#define uuid_s_ok                       error_status_ok
#define uuid_s_bad_version              (382312584)
#define uuid_s_socket_failure           (382312585)
#define uuid_s_getconf_failure          (382312586)
#define uuid_s_no_address               (382312587)
#define uuid_s_overrun                  (382312588)
#define uuid_s_internal_error           (382312589)
#define uuid_s_coding_error             (382312590)
#define uuid_s_invalid_string_uuid      (382312591)
#define uuid_s_no_memory                (382312592)
#define utils_s_802_cant_read           (0x1460101e)
#define utils_s_802_addr_format         (0x1460101f)
#define utils_s_802_uname_failure       (0x14601020)


      /** A type */
typedef struct  {
  unsigned int count;
  uuid_t * uuid;
} uuid_vector_t;

typedef uuid_vector_t *uuid_vector_p_t;

/*========================================================================
 * PUBLIC INTERFACES TO THE UUID LIBRARY
 *========================================================================
 */

extern void uuid_create(uuid_t *uuid,
			unsigned32 *status
			);

extern void uuid_create_nil(uuid_t *uuid, 
			    unsigned32 *status
			    );

extern void uuid_to_string(uuid_p_t uuid,
			   unsigned_char_p_t * uuid_string,
			   unsigned32 *status
			   );

extern void uuid_from_string(unsigned_char_p_t uuid_string,
			     uuid_t *uuid,
			     unsigned32 *status
			    );

extern boolean32 uuid_equal(uuid_p_t uuid1,
			    uuid_p_t uuid2,
			    unsigned32 *status
			    );

extern boolean32 uuid_is_nil(uuid_p_t uuid,
			     unsigned32 *status
			     );

extern signed32 uuid_compare(uuid_p_t uuid1,
			     uuid_p_t uuid2,
			     unsigned32 *status
			     );

extern unsigned16 uuid_hash(uuid_p_t uuid,
			    unsigned32 *status
			    );

#ifdef __cplusplus
    }
#endif








