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
#ifndef COMPFRAME_IFACE_M_H
#define COMPFRAME_IFACE_M_H

#include "compframe_types.h"

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */
#if 0
}
#endif
/** @addtogroup ifaces CompFrame Interfaces 
    @{
*/
/** Textual name of the M (Message Transport Component) server interface,
    that is implemented by the M component. */
#define CF_M_SERVER_IFACE_NAME  "cfi_m_server"
/** Interface type */
typedef struct cfi_m_server {
    /** Add a message receiver
        @param m        Pointer to M component
        @param comp     Pointer to own context
        @param uuid     UUID string for the interface 
        @param name     Name of message receiver
        @param userData User's data
        @return 1 if OK, 0 if failure
    */
    int (*mr_add) (void *m, void *comp, const char *uuid,
                   char *name, void *userData);

    /** Enable message receiver, i.e allow connections to this receiver.
        @note Receivers are enabled by default.
        @param m    Pointer to M component
        @param uuid UUID string for the interface 
        @param name Name of message receiver
        @return 1 if OK, 0 if failure
    */
    int (*mr_enable) (void *m, const char *uuid, char *name);

    /** Disable message receiver, i.e do not allow any more connections
        to this receiver.
        @param m    Pointer to M component
        @param uuid UUID string for the interface 
        @param name Name of message receiver
        @return 1 if OK, 0 if failure
    */
    int (*mr_disable) (void *m, const char *uuid, char *name);

    /** Remove message receiver
        @param m    Pointer to M component
        @param uuid UUID string of the interface 
        @param name Name of message receiver
        @return 1 if OK, 0 if failure
    */
    int (*mr_del) (void *m, const char *uuid, char *name);

    /** Returns the port of the M server 
        @param m    Pointer to M component
        @return the port number of -1 if not started.
    */
    int (*port_get) (void *m);

    /** Returns a colon separated string with message receivers
        with specific names (name="M" will return all that start
        with 'M')
        @param m    Pointer to M component
        @param name String with name or prefix.
        @return the port number of -1 if not started.
    */
    char *(*search_by_name) (void *m, char *name);

    /** Returns a colon separated string with message receivers
        with a specific interface.
        @param m    Pointer to M component
        @param uuid UUID string of interface.
        @return the port number of -1 if not started.
    */
    char *(*search_by_if) (void *m, const char *uuid);

    /** Send a message to a receiver 

     */
    int (*send) (void *m, void *conn, uint8_t chan, int len,
                 unsigned char *msg);

} cfi_m_server;

/** Textual name of the M (Message Transport Component) client interface,
    that must be  implemented by the M client component. */
#define CF_M_CLIENT_IFACE_NAME  "cfi_m_client"

/** Interface type */
typedef struct cfi_m_client {
    /** Called when a connection is made to the message receiver  
        @param comp     Component context
        @param conn     Pointer to connection context
        @param chan     Channel number
        @param userData User's own data. (From mr_add())
        @return 1 if OK, 0 if failure
    */
    int (*connected) (void *comp, void *conn, uint8_t chan, void *userData);

    /** Called when a connection closed down for the message receiver  
        @param comp     Component context
        @param conn     Pointer to connection context
        @param chan     Channel number
        @param userData User's own data. (From mr_add())
        @return 1 if OK, 0 if failure
    */
    int (*disconnected) (void *comp, void *conn, uint8_t chan, void *userData);

    /** Called to send a message to a component
        @param comp     Component context
        @param conn     Pointer to connection context
        @param chan     Channel number
        @param len      Length of message
        @param msg      Pointer to message
        @param userData User's own data. (From mr_add())
        @return 1 if OK, 0 if failure
    */
    int (*message) (void *comp,
                    void *conn,
                    uint8_t chan, int len, unsigned char *msg, void *userData);
} cfi_m_client;

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

/** @}   Doxygen end marker */

/** @addtogroup m M - Message Transport
 *  @{
 */

/*============================================================================*/
/* MACROS                                                                     */
/*============================================================================*/

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

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* COMPFRAME_IFACE_M_H */
