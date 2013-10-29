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
#ifndef COMPFRAME_SOCKETS_H
#define COMPFRAME_SOCKETS_H

/*===========================================================================*/
/* INCLUDES                                                                  */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */
#if 0
}
#endif
/*===========================================================================*/
/* PUBLIC FUNCTION DECLARATIONS                                              */
/*===========================================================================*/
/** @addtogroup Public API
 *  These functions are part of the public API of CompFrame
 *  @{
 */
/*---------------------------------------------------------------------------*/
/* SOCKET FUNCTIONS                                                          */
/*---------------------------------------------------------------------------*/
/** Used to register a socket for polling in the global polling function.
    @param comp     Own context
    @param sd       Socket descriptor
    @param fp       Function pointer to callback
    @param userData User data that will be passed in the callback
    @return 1 if OK, 0 if failure
*/
int
cf_socket_register(void *comp, int sd, cf_sock_callback_t fp, void *userData);

/** Used to deregister a socket for polling in the global polling function.
    @param sd       Socket descriptor
    @return 1 if OK, 0 if failure
*/
int
cf_socket_deregister(int sd);

/** Connects a socket to a port on a specified host.
    @param host Host name
    @param port Port number
    @param sd   Socket descriptor (returned)
    @return 0 if OK, -1 if failure
*/
int
cf_socket_connect(char *host, int port, int *sd);

/** Creates a server socket.
    @param port Desired port number
    @return The socket descriptor or -1 if failure
*/
int
cf_socket_server(int port);

/** Makes sure all bytes are written to a socket.
    @param sd   Socket descriptor
    @param buf  Buffer to write
    @param n    Number of bytes to write
    @return If OK greater than Zero, if not -1
*/
int
cf_socket_write(int sd, unsigned char *buf, size_t n);

/** Returns the host and port number for a socket.
    @param sd   Socket descriptor
    @param host Host name (returned)
    @param port Port number (returned)
    @return 0 if OK, -1 if not
*/
int
cf_socket_name_get(int sd, char **host, int *port);

/** Returns the port number for a socket.
    @param sd   Socket descriptor
    @return The port number, or -1 if failure
*/
int
cf_socket_port_get(int sd);

/** Returns number of bytes available for read on a socket
    @param sd   Socket descriptor
    @return The number of bytes, or -1 if failure
*/
int
cf_socket_bytes_to_read(int sd);

/** Sets a socket as reusable
    @param sd   Socket descriptor
    @return 0 if OK, -1 if failure
*/
int
cf_socket_reusable(int sd);

/** Polls all the sockets  */
int
cf_sockets_poll(void);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* COMPFRAME_H */
