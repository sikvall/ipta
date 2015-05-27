/**********************************************************************
 * Anders "Ichimusai" Sikvall
 * anders@sikvall.se
 *
 * This header file is part of the ipta package and is maintained by
 * the package owner, see http://ichimusai.org/ipta/ for more info
 * about this. Any changes, patches, diffs etc that you would like to
 * offer should be sent by email to ichi@ichimusai.org for review
 * before they will be applied to the main code base.
 *
 * As usual this software is offered "as is" and placed in the public
 * domain. You are free to copy, modify, spread and make use of this
 * software. For the terms and conditions for this software you
 * should refer to the "LICENCE" file in the source directory or run
 * a compiled binary with the "--licence" option which will display
 * the licence.
 *
 * Any modifications to this source MUST retain this header. You are
 * however allowed to add below your own changes and redistribute, as
 * long as you do not violate any terms and condition in the LICENCE.
 **********************************************************************/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "ipta.h"

/***********************************************************************
 * The follow function will follow the file given as argument and
 * interprete it and print each logged packet line by line without
 * storing anything in the database. It's just a realtime look at what
 * is logged in a nicely formatted way.
 **********************************************************************/

int follow(char *filename, struct ipta_flags *flags) {
	FILE *logfile;
	int flag_rdns = FLAG_CLEAR;
	char *line;
	size_t len = HOSTNAME_MAX_LEN;
	ssize_t read;
	char prefix[]=IPTA_LINE_PREFIX;
	char *log_action;
	char *log_ifin;
	char *log_ifout;
	char *log_mac;
	char *log_src;
	char *log_dst;
	char *log_proto;
	char *log_src_port;
	char *log_dst_port;
	char *dummy;
	char *token;
	char nullstring[] = "";
	int line_count = 0;
	int packet_count = 0;
	char src_hostname[HOSTNAME_MAX_LEN];
	char dst_hostname[HOSTNAME_MAX_LEN];
	int hostname_len = 30;
	int retval = RETVAL_OK;

	line = calloc(256, 1);

	logfile = fopen(filename, "r");
	if(!logfile) {
		fprintf(stderr, "! ERROR: Unable to open the file %s.\n", filename);
		retval = RETVAL_ERROR;
		goto clean_exit;
	}

	/* Seek to the end of the file unless we have a flag to show the
	   history as well */
	retval = fseek(logfile, 0L, SEEK_END);
	if(0 != retval) {
		fprintf(stderr, "! Error seeking to end of log file. Exiting.\n");
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
  

	/* Actually this goes on until CTRL-C is pressed, so we will
	   actually never return from this function once we started the
	   following. */
  
	while (1) {
		read = getline(&line, &len, logfile);

		if(read == -1) {

			/* If the user wants rdns we turn it on only after reaching EOF
			   first. Otherwise it can take forever if we have a large log
			   file to read through. */

			if(flags->rdns)
				flag_rdns = FLAG_SET;
			sleep(1);
			continue;
		} else {
			/* We only want lines that contains the prefix */
			if(strstr(line, prefix)) {
				dummy = strtok(line, " ");
	
				/* Wind until marker found */
				do {
					dummy = strtok(NULL, " ");
				} while (strcmp(dummy, "IPT:"));
	
				log_action = strtok(NULL, " ");

				packet_count++;
	
				/* Clear records for next run */
				log_ifin = log_ifout = log_mac = log_src = log_dst = 
					log_proto = log_src_port = log_dst_port = nullstring;
		
				/* In here we process the known prefixes and
				 * the data * from each field in the log
				 * string. Fields that are * not known are
				 * just ignored silently. Anything that is
				 * changed or added here needs to reflect the
				 * database */
	
				while ((token = strtok(NULL, " "))) {
					if(!strncmp("ACTION=", token, 7)) {
						log_action = token + strlen("ACTION=");
						continue;
					}
					if(!strncmp("IN=", token, 3)) {
						log_ifin = token + strlen("IN=");
						continue;
					}
					if(!strncmp("OUT=", token, 4)) {
						log_ifout = token + strlen("OUT=");
						continue;
					}
					if(!strncmp("MAC=", token, 4)) {
						log_mac = token + strlen("MAC=");
						continue;
					}
					if(!strncmp("SRC=", token, 4)) {
						log_src = token + strlen("SRC=");
						if(flag_rdns) {
							retval = get_host_by_addr(log_src, src_hostname, hostname_len);
							if(retval != 0) 
								strcpy(src_hostname, log_src);
						}
						continue;
					}
					if(!strncmp("DST=", token, 4)) {
						log_dst = token + strlen("DST=");
						if(flag_rdns) {
							retval = get_host_by_addr(log_dst, dst_hostname, hostname_len);
							if(retval != 0) 
								strcpy(src_hostname, log_dst);
						}
						continue;
					}
					if(!strncmp("PROTO=", token, 6)) {
						log_proto = token + strlen("PROTO=");
						continue;
					}
					if(!strncmp("SPT=", token, 4)) {
						log_src_port = token + strlen("SPT=");
						continue;
					}
					if(!strncmp("DPT=", token, 4)) {
						log_dst_port = token + strlen("DPT=");
						continue;
					}
				}

				if(!(flags->no_lo && (!strcmp(log_ifin, "lo") || !strcmp(log_ifout, "lo")))) {
					if(!(flags->no_accept && !strcmp("ACCEPT", log_action))) {
	
						/* Time to print the line in a nice formatted way */
						if(line_count == 0) {
							printf("\n");
							printf("Count    IF       Source                          Port Destination                     Port Proto      Action    \n");
							printf("-------- -------- ------------------------------ ----- ------------------------------ ----- ---------- ----------\n");
						}
	
						line_count++;
	
						if ( line_count >= 20 ) 
							line_count = 0;
	
						printf("%8d %-8s %-30s %5d %-30s %5d %-10s %-10s\n",
						       packet_count,
						       strcmp("", log_ifin) ? log_ifin : log_ifout,
						       flag_rdns ? src_hostname : log_src,
						       atoi(log_src_port),
						       flag_rdns ? dst_hostname : log_dst,
						       atoi(log_dst_port),
						       log_proto,
						       log_action);
					}
				}
			}
		}
	}
	
clean_exit:
	fcloseall();
	free(line);
	return retval;
}
  

