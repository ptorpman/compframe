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
#ifndef COMPFRAME_REGISTRY_H
#define COMPFRAME_REGISTRY_H

/* I N T E R N A L  S T U F F */

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */
#if 0
}
#endif
/*===========================================================================*/
/* MACROS                                                                    */
/*===========================================================================*/
/*===========================================================================*/
/* TYPES                                                                     */
/*===========================================================================*/

/*===========================================================================*/
/* FUNCTION DECLARATIONS                                                     */
/*===========================================================================*/

/** Parses a command from the command line or configuration file 
 *  @param usr  Command line
 *  @return User defined (Suggest 0 for failure, 1 for success) 
 */
int
cf_command_handle(char *usr);


/** Polls all the sockets  */
int
cf_sockets_poll(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* COMPFRAME_REGISTRY_H */
