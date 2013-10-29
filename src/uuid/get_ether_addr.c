/*
  dce_802_addr.c for Linux 2.x
 
  Jim Doyle, Boston University, Jan 17 1998

  Fetch a IEEE 802 MAC Address for building UUID's

*/



#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#ifndef UUID_BUILD_STANDALONE
#include <dce/dce.h>
#include <dce/dce_utils.h>
#else
#include "uuid.h"
#include "dce_utils.h"          /* defines  dce_get_802_addr()          */
#endif
#include <sys/ioctl.h>          

#include <sys/socket.h>

#ifdef SOLARIS
#include <sys/sockio.h>
#endif

#include <net/if.h>
#include <net/if_arp.h>

#include <stdlib.h>

/*
** Invalid Host MAC Addresses
*/

static  char null_802_hwaddr[6] = {0, 0, 0, 0, 0, 0};

static void solaris_get_802_addr(dce_802_addr_t *, error_status_t *);
static void file_get_802_addr(dce_802_addr_t *, error_status_t *);

void
dce_get_802_addr (
    dce_802_addr_t	*addr,
    error_status_t	*st
)
{

  /* Try to get a MAC address from one of our Ethernet interfaces */

  solaris_get_802_addr(addr, st);

  /* If there are no Ethernet-style interfaces, fall-back and try to
     fetch the address from a file. */

  if (*st != error_status_ok)
    {
      file_get_802_addr(addr, st);
    }  
}

int solaris_get_hwaddr(int afinet_socket,
                       const char *interface_name,
                       dce_802_addr_t *addr)
{
  /* Find out the MAC address of the interface_name from the
     afinet_socket.  If that's not available, return false. */

  struct ifreq ifr;
  struct arpreq arpr;

  /* FIXME: If more than IFNAMSIZ characters are copied, the below
     line will overflow. */
  strcpy (ifr.ifr_name, interface_name);
  
  /* Find the IP address of the afinet_socket */
  if (ioctl (afinet_socket, SIOCGIFADDR, &ifr) < 0)
    {
      return 0;  /* Interface IP address not available */
    }

  /* Do an ARP request for the afinet_socket */
  memcpy(&(arpr.arp_pa), &(ifr.ifr_addr), sizeof(struct sockaddr));

  if (ioctl (afinet_socket, SIOCGARP, &arpr) < 0)
    {
      return 0;  /* Interface MAC address not available */
    }

  memcpy(addr->eaddr, arpr.arp_ha.sa_data, 6);

  return 1;  /* Success!! */
}

/*
** Fetch a valid MAC address from one of the host's Ethernet interfaces
*/

static void 
solaris_get_802_addr(
    dce_802_addr_t	*addr,
    error_status_t	*st
    )
{
  int    afinet_socket;

  struct ifconf           ifc;
  struct ifreq            *ifr, *last_ifr;
  unsigned char           buf[1024]; 
  int                     n_ifs, i; 
  int                     prev_size = sizeof(struct ifreq); 

  int continue_search;

  *st = utils_s_802_cant_read;

  /*
  ** Open an INET socket
  */

  afinet_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (afinet_socket < 0)
    {
      printf ("Cant open socket\n");
      exit(1);
    }

  /*
  ** Get the list of network interfaces
  */

  ifc.ifc_len = sizeof (buf);
  ifc.ifc_buf = (caddr_t) buf;

  if (ioctl (afinet_socket, (int) SIOCGIFCONF, (caddr_t) &ifc) < 0)
    {                                                 
      printf ("ioctl(SIOCGIFCONF) failed\n");
      exit(EXIT_FAILURE);

    }

  n_ifs = ifc.ifc_len / sizeof (struct ifreq); 
  last_ifr = (struct ifreq *) (ifc.ifc_buf + ifc.ifc_len);  

  /*
  ** Find an interface that has a valid IEEE 802 address
  **
  ** Iterate of the list of the system's netif. Find the first
  ** active interface that has an Ethernet address ; use it.
  */

  continue_search = 1;

  for (i=0, ifr = ifc.ifc_req; 
       (ifr < last_ifr) && continue_search;
       i++, ifr = (struct ifreq *)(( (char *) ifr ) + prev_size))          
    {
      /*
      ** Get Flags for this interface
      */

      continue_search = ! solaris_get_hwaddr(afinet_socket,
                                             ifr->ifr_name,
                                             addr);
    }

  if (memcmp(addr, null_802_hwaddr, 6) == 0)
    {
      /* FIXME: The MAC address is 0:0:0:0:0:0.  Should we fail
         because of this?  Returning here means "yes", but I don't
         know if that is the correct behaviour. */

      return;
    }

  close(afinet_socket);

  *st = error_status_ok;
}

/*
** Fetch a dummy MAC address from the file /etc/ieee_802_addr
*/

static void
file_get_802_addr (
    dce_802_addr_t	*addr,
    error_status_t	*st
)
{
    int		fd;
    char	buf[13];
    int		e[6];
    int		i;

    *st = error_status_ok;

    fd = open (IEEE_802_FILE, O_RDONLY);
    if (fd < 0) {
	*st = utils_s_802_cant_read;
	return;
    }

    if (read (fd, buf, 12) < 12) {
	*st = utils_s_802_cant_read;
	return;
    }
    close(fd);

    buf[12] = 0;

    if (sscanf (buf, "%2x%2x%2x%2x%2x%2x", 
		&e[0], &e[1], &e[2], &e[3], &e[4], &e[5]) != 6) {
	*st = utils_s_802_addr_format;
	return;
    }

    for (i = 0; i < 6; i++)
       addr->eaddr[i] = e[i];
}

void assert(boolean foo) { (void)foo; }
