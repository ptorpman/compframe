/* Copyright (c) 2007  Peter R. Torpman (peter at torpman dot se)

   This file is part of CompFrame (http://compframe.sourceforge.net)

   CompFrame is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   CompFrame is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.or/licenses/>.
*/
#ifndef COMPFRAME_M_H
#define COMPFRAME_M_H

/*============================================================================*/
/* INCLUDES                                                                   */
/*============================================================================*/

#include "compframe.h"
#include "compframe_iface_m.h"

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */
#if 0
}
#endif
/*============================================================================*/
/* TYPES                                                                      */
/*============================================================================*/
/** Used for keeping track of users of a specified connection */
typedef struct cfm_peer_t {
    /** Socket descriptor */
    int sd;
    /**  Channel used */
    int channel;
    /** Pointer to locally connected receiver  */
    void *localReceiver;
    /** Will be called when connection is opened */
    cfm_callback_open_t callback_open;
    /** Will be called when connection is closed */
    cfm_callback_close_t callback_close;
    /** Will be called when connection has a fault */
    cfm_callback_error_t callback_error;
    /** Will be called when message is received */
    cfm_callback_msg_t callback_msg;
    /** Will be returned to user in callbacks */
    void *userData;
} cfm_peer_t;

/** Type used for keeping track of the stuff going on for a specific
    connection */
typedef enum {
    CFM_CONN_INIT = 0,
    CFM_CONN_CHANNEL = 1,
    CFM_CONN_LENGTH1 = 2,
    CFM_CONN_LENGTH2 = 3,
    CFM_CONN_MSGBODY = 4,
    CFM_CONN_MSGREADY = 5
} cfm_conn_state_t;

/** Used for M connections */
struct cfm_conn_t {
    /** Pointer to next  */
    struct cfm_conn_t *next;
    /** Pointer to previous  */
    struct cfm_conn_t *prev;

    /** Host name*/
    char *host;
    /** Port number */
    int port;

    /** Socket descriptor  */
    int socket_fd;
    /** Number of used channels (Range 0..255) */
    uint8_t num_used;
    /** Peers on this connection  */
    cfm_peer_t *peer[CFM_MAX_PEERS];
    /** State of buffer  */
    cfm_conn_state_t state;

    /** Flag if remote connection is a M compoent  */
    int isM;
    /** Current channel handled  */
    int channel;
    /** message length */
    int msg_len;
    /** Message buffer  */
    unsigned char *msg_buff;
    /** Message position */
    int msg_pos;
};

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* COMPFRAME_M_H */
