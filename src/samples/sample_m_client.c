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

/*===========================================================================*/
/* INCLUDES                                                                  */
/*===========================================================================*/

#include "compframe_m_lib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/poll.h>

/*===========================================================================*/
/* MACROS                                                                    */
/*===========================================================================*/

/*===========================================================================*/
/* TYPES                                                                     */
/*===========================================================================*/

/*===========================================================================*/
/* VARIABLES                                                                 */
/*===========================================================================*/


/*===========================================================================*/
/* FUNCTION DECLARATIONS                                                     */
/*===========================================================================*/


static int 
myOpenCB(void* conn, int chan,void* userData);

static int 
myCloseCB(void* conn, int chan,void* userData);

static int
myErrorCB(void* conn, char* errorMsg, void* userData);

static int 
myMsgCB(void* conn, int chan,int len, unsigned char* msg, void* userData);


/*===========================================================================*/
/* FUNCTION DEFINITIONS                                                      */
/*===========================================================================*/

int main(int argc, char** argv)
{
  struct pollfd fds[2];

  if (argc != 4) {
    fprintf(stderr,"Usage: sample_m_client <host> <port> <receiver>\n");
    return 1;
  }

  /* First, open a connection to M in remote process */
  void* conn = cfm_connection_open(argv[1],atoi(argv[2]));

  if (!conn) {
    fprintf(stderr,"Connection could not be established\n");
    return 1;
  }

/* (See sampl2.c) */
#define TEST_UUID "433e76d0-77d1-460d-9321-e2dc8dc8bd59"

  /* Second, open a channel to a receiver in remote process */
  int channel = 
    cfm_channel_open(conn,
                     TEST_UUID,
                     argv[3],
                     myOpenCB,
                     myCloseCB,
                     myMsgCB,
                     myErrorCB,
                     NULL);
                     
  if (channel == -1) {
    fprintf(stderr,"Channel could not be opened\n");
    cfm_connection_close(conn);
  }
  
  channel = cfm_channel_open(conn,
                             TEST_UUID,
                             argv[3],
                             myOpenCB,
                             myCloseCB,
                             myMsgCB,
                             myErrorCB,
                             NULL);

  if (channel == -1) {
    fprintf(stderr,"Channel could not be opened\n");
    cfm_connection_close(conn);
  }


  fds[0].fd     = cfm_connection_sd_get(conn);
  fds[0].events = POLLIN;

  int res;

  while (1) {
    res = poll(fds, 1, 10);
    
    if (fds[0].revents == POLLIN) {
      if (cfm_sockets_handle(fds[0].fd) < 0) {
        conn = cfm_connection_find(fds[0].fd);
        cfm_connection_close(conn);
        fprintf(stderr,"Info: Connection closed!\n");
        return 1;
      }


    }
  }

  return 0;
}



static int 
myOpenCB(void* conn, int chan,void* userData)
{
  fprintf(stdout,
          "Connection is open! %p chan=%d user=%p\n",
          conn, chan, userData);

  char msg[256];
  strcpy(msg, "Hello, sample2!\n");
  
  return cfm_message_send(conn,chan, strlen(msg) + 1, (unsigned char*)msg );
}


static int 
myCloseCB(void* conn, int chan,void* userData)
{
  fprintf(stdout,
          "Connection is closed! %p chan=%d user=%p\n",
          conn, chan, userData);

  return 1;
}

static int
myErrorCB(void* conn, char* errorMsg, void* userData)
{
  fprintf(stdout,
          "Connection has errors! %p msg=%s user=%p\n",
          conn, 
          (errorMsg ? errorMsg : "NONE"), 
          userData);
  exit(1);
}

static int 
myMsgCB(void* conn, int chan,int len, unsigned char* msg, void* userData)
{
  fprintf(stderr,
          "Got a message! conn=%p chan=%d user=%p len=%d\n"
          "MSG: >%s<\n",
          conn, chan, userData,len, msg);


  /* Close channel */
  cfm_channel_close(conn,chan);
  cfm_connection_close(conn);
  exit(1);

  /* Not reached! */
  return 1;

}

