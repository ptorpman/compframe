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
#ifndef COMPFRAME_M_LIB_H
#define COMPFRAME_M_LIB_H

#include "compframe_types.h"

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */
#if 0
}
#endif
/** @addtogroup mcli M Client Library 
 *  @{
 */
/*===========================================================================*/
/* MACROS                                                                    */
/*===========================================================================*/
/** Channel number used for M server to/from client communication   */
#define CFM_M_CHANNEL 255

/** Maximum number of channels */
#define CFM_MAX_CHANNELS 255

/** Maximum total length of an M message */
#define CF_M_MAX_MESSAGE 2048

/*---- Commands used by clients towards M ----*/

/** Message used for opening a channel towards a receiver
    @verbatim
    +------+--------+--------+-------+----------+----------+---+
    | CHAN | LEN LB | LEN HB | ORDER | UUID     | NAME     | 0 |
    +------+--------+--------+-------+----------+----------+---+
    CHAN   - 1 byte (Here M command channel)
    LEN LB - 1 byte (Total length low byte)
    LEN HB - 1 byte (Total length high byte)
    ORDER  - 1 byte (CF_M_CHANNEL_OPEN)
    UUID   - 36 bytes
    NAME   - n  bytes
    @endverbatim
*/
#define CF_M_CHANNEL_OPEN  0

/** Message used for closing a channel towards a receiver
    @verbatim
    +------+--------+--------+-------+-------------+
    | CHAN | LEN LB | LEN HB | ORDER | CHANTOCLOSE |
    +------+--------+--------+-------+-------------+
    CHAN        - 1 byte (Here M command channel)
    LEN LB      - 1 byte (Total length low byte)
    LEN HB      - 1 byte (Total length high byte)
    ORDER       - 1 byte (CF_M_CHANNEL_CLOSE)
    CHANTOCLOSE - 1 byte (Channel to close)
    @endverbatim
*/
#define CF_M_CHANNEL_CLOSE 1

/** Message used for sending a message towards a receiver
    @verbatim
    +------+--------+--------+---------+
    | CHAN | LEN LB | LEN HB | MESSAGE |
    +------+--------+--------+---------+
    CHAN    - 1 byte
    LEN LB  - 1 byte (Total length low byte)
    LEN HB  - 1 byte (Total length high byte)
    MESSAGE - n  bytes
    @endverbatim
*/
#define CF_M_MESSAGE_SEND 2

/** Message used for indicating that a channel could be opened 
    @verbatim
    +------+--------+--------+-------+----------+---------------+---+
    | CHAN | LEN LB | LEN HB | ORDER | RESPONSE | RESPONSE TEXT | 0 |
    +------+--------+--------+-------+----------+---------------+---+
    CHAN          - 1 byte
    LEN LB        - 1 byte (Total length low byte)
    LEN HB        - 1 byte (Total length high byte)
    ORDER         - 1  byte
    RESPONSE      - 1 byte
    RESPONSE TEXT - n bytes
    @endverbatim
*/
#define CF_M_CHANNEL_OPEN_OK    3

/** Message used for indicating that a channel could not be opened 
    @verbatim
    +------+--------+--------+-------+----------+---------------+---+
    | CHAN | LEN LB | LEN HB | ORDER | RESPONSE | RESPONSE TEXT | 0 |
    +------+--------+--------+-------+----------+---------------+---+
    CHAN          - 1 byte
    LEN LB        - 1 byte (Total length low byte)
    LEN HB        - 1 byte (Total length high byte)
    ORDER         - 1  byte
    RESPONSE      - 1 byte
    RESPONSE TEXT - n bytes
    @endverbatim
*/
#define CF_M_CHANNEL_OPEN_FAIL  4

/** Message used for indicating that a channel could be closed 
    @verbatim
    +------+--------+--------+-------+----------+---------------+---+
    | CHAN | LEN LB | LEN HB | ORDER | RESPONSE | RESPONSE TEXT | 0 |
    +------+--------+--------+-------+----------+---------------+---+
    CHAN          - 1 byte
    LEN LB        - 1 byte (Total length low byte)
    LEN HB        - 1 byte (Total length high byte)
    ORDER         - 1  byte
    RESPONSE      - 1 byte
    RESPONSE TEXT - n bytes
    @endverbatim
*/
#define CF_M_CHANNEL_CLOSE_OK   5
/** Message used for indicating that a channel could not be closed 
    @verbatim
    +------+--------+--------+-------+----------+---------------+---+
    | CHAN | LEN LB | LEN HB | ORDER | RESPONSE | RESPONSE TEXT | 0 |
    +------+--------+--------+-------+----------+---------------+---+
    CHAN          - 1 byte
    LEN LB        - 1 byte (Total length low byte)
    LEN HB        - 1 byte (Total length high byte)
    ORDER         - 1  byte
    RESPONSE      - 1 byte
    RESPONSE TEXT - n bytes
    @endverbatim
*/
#define CF_M_CHANNEL_CLOSE_FAIL 6

/** Message used for sending a response for wrong command used
    @verbatim
    +------+--------+--------+-------+-----+----------+---------------+---+
    | CHAN | LEN LB | LEN HB | ORDER | RES | RESPONSE | RESPONSE TEXT | 0 |
    +------+--------+--------+-------+-----+----------+---------------+---+
    CHAN          - 1 byte
    LEN LB        - 1 byte (Total length low byte)
    LEN HB        - 1 byte (Total length high byte)
    ORDER         - 1 byte
    RES           - 1 byte
    RESPONSE      - 1 byte
    RESPONSE TEXT - n bytes
    @endverbatim
*/
#define CF_M_CHANNEL_ORDER_UNKNOWN 7

/** Error code for 'component not found' */
#define CF_M_COMP_NOT_FOUND  100
/** Error code for 'Out of channels' */
#define CF_M_OUT_OF_CHANNELS 101
/** Error code for 'Could not connect' */
#define CF_M_COULD_NOT_CONNECT 102

/** @}   Doxygen end marker */

/*===========================================================================*/
/* TYPES                                                                     */
/*===========================================================================*/

/** Forward declaration */
typedef struct cfm_conn_t cfm_conn_t;

/** Callback used when telling a user that a connection has been established.
 */
typedef int
(*cfm_callback_open_t) (void *conn, int chan, void *userData);

/** Callback used when telling a user that a connection has been closed.
 */
typedef int
(*cfm_callback_close_t) (void *conn, int chan, void *userData);

/** Callback used when telling a user that a connection has an error.
    And could not be established.
*/
typedef int
(*cfm_callback_error_t) (void *conn, char *errorMsg, void *userData);

/** Callback used when telling a user that a message has been received.
 */
typedef int
(*cfm_callback_msg_t) (void *conn, int chan,
                       int len, unsigned char *msg, void *userData);

/*===========================================================================*/
/* FUNCTION DECLARATION                                                      */
/*===========================================================================*/

/** Opens a connection to an M server
    @param host  M host name
    @param port  M port number
    @return Pointer to connection or NULL*/
void *cfm_connection_open(char *host, int port);

/** Returns the socket descriptor for an M connection,
    to be used in polling.
    @param conn Pointer to connection
    @return Socket descriptor or -1
*/
int
cfm_connection_sd_get(void *conn);

/** Closes a connection to an M server
    @param conn Pointer to connection
    @return 1 if OK, 0 if not.
*/
int
cfm_connection_close(void *conn);

/** Opens a channel to an M receiver
    @param conn Pointer to connection
    @param uuid ID of interface
    @param name Name of receiver
    @param openCB  Callback that is called when connection is established
    @param closeCB Callback that is called when connection is closed
    @param msgCB   Callback that is called when a message is received
    @param errorCB Callback that is called when an error occurs
    @param userData User specific data.
    @return 1 if OK, 0 if not
*/
int
cfm_channel_open(void *conn,
                 const char *uiid,
                 char *name,
                 cfm_callback_open_t openCB,
                 cfm_callback_close_t closeCB,
                 cfm_callback_msg_t msgCB,
                 cfm_callback_error_t errorCB, void *userData);

/** Opens a channel to an M receiver
    @param conn Pointer to connection
    @param chan Channel number
    @return 1 if OK, 0 if not
*/
int
cfm_channel_close(void *conn, int chan);

/** Sends a message to an M receiver
    @param conn Pointer to connection
    @param chan Channel number
    @param len  Length of message
    @param msg  Message
    @return 1 if OK, 0 if not
*/
int
cfm_message_send(void *conn, int chan, int len, unsigned char *msg);

/** Handles pending socket events for a connection socket
    @param sd Socket descriptor
    @return 1 if OK, 0 if not.
*/
int
cfm_sockets_handle(int sd);

/** Returns the M connection for a specific socket descriptor
    @param sd Socket descriptor
    @return Pointer to connection, or NULL.
*/
void *cfm_connection_find(int sd);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* COMPFRAME_M_LIB_H */
