/**********************************************************************
 * gethostbyaddr.c
 *
 * Anders "Ichimusai" Sikvall
 * anders@sikvall.se
 *
 * This source file is part of the ipta package and is maintained by
 * the package owner, see http://ichimusai.org/ipta/ for more info
 * about this. Any changes, patches, diffs etc that you would like to
 * offer should be sent by email to ichi@ichimusai.org for review
 * before they will be applied to the main code base.
 *
 * As usual this software is offered "as is" and placed in the public
 * domain. You are free to copy, modify, spread and make use of this
 * software. For the terms and conditions for this software you should
 * refer to the "LICENCE" file in the source directory or run a
 * compiled binary with the "--licence" option which will display the
 * licence.
 *
 * Any modifications to this source MUST retain this header. You are
 * however allowed to add below your own changes and redistribute, as
 * long as you do not violate any terms and condition in the LICENCE.
 **********************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


/* 
 * Get host by address
 * 
 * ip_address      char *      The string containing the IP adrress in dotted format
 *                             that is 192.168.0.1
 * hostname        char *      A pointer to a char where hostname will be stored,
 *                             watch your buffer oversuns here and make sure you
 *                             use the maxlen feature, thanks.
 * maxlen          int         Maximum length of hostname, it is truncated from the
 *                             beginning so that important hostnames, networks and
 *                             tld is preserved.
 */
int get_host_by_addr(char *ip_address, char *hostname, int maxlen) {
  struct sockaddr_in ip4addr;
  int len = 0;
  char host[NI_MAXHOST];        // Holding hostname
  char service[NI_MAXSERV];     // Holding service
  int retval = 0;

  /* Clear all stuff */
  memset(&ip4addr, 0, sizeof(struct sockaddr_in));

  /* Set up family and port */
  ip4addr.sin_family = AF_INET;
  ip4addr.sin_port = htons(0);

  /* Convert the address from string to AF_INET proper address and
     populate structure with it. */
  inet_pton(AF_INET, ip_address, &ip4addr.sin_addr);

  /* Call the DNS subsystem */
  int s = getnameinfo((struct sockaddr *) &ip4addr, sizeof(struct sockaddr_in), 
		      host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);

  /* If we got a name then we copy the name to maxlen characters into
     the hostname pointer provided by the call. */
  if (s == 0) {
    len = strlen(host);
    if(len > maxlen) {
      strcpy(host, host+(len-maxlen));
      host[0] = '*';
    }
    sprintf(hostname, "%s", host);
    return 0;
  }
  else {

    /* If not, then we return the actual IP address. We also signal
       that we could not look it up with a "1" as RETVAL Return the
       address cut so that it fits the field length */

    strncpy(hostname, ip_address, maxlen);

    return 1; /* Nor regarded as a fatal error, just a signal in
                 RETVAL that we did not look up a name */
  }
}
