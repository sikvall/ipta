/**********************************************************************
 * import-syslog.c
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

#include <my_global.h>
#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "ipta.h"

int import_syslog(struct ipta_db_info *db_info, char *filename)
{
	FILE *logfile = NULL;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
//  char *month_string = NULL;
//  char *day_string = NULL;
//  char *hour_string = NULL;
//  char *minute_string = NULL;
//  char *second_string= NULL;
	/* int month, day, hour, minute,second = 0; */
	/* char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",  */
	/* 		     "Aug", "Sep", "Oct", "Nov", "Dec" }; */
	//  char **saveptr = NULL;
//  int i;
//  char *dummyptr = NULL;
	int lines = 0;
	int row_counter = 0;
	time_t starttime = 0;
	//  int worktime = 0;
	//  char *host = NULL;
	char *module = NULL;
	//  char *host_time = NULL;
	char prefix[]="IPT: ";
	char *token = NULL;
	char *log_action = NULL;
	//  char *log_prefix = NULL;
	char *log_ifin = NULL;
	char *log_ifout = NULL;
	char *log_mac = NULL;
	char *log_src = NULL;
	char *log_dst = NULL;
	//  char *log_len = NULL;
	char *log_proto = NULL;
	char *log_src_port = NULL;
	char *log_dst_port = NULL;
	char nullstring[] = "";
	MYSQL *con = NULL;
	char *query_string = NULL;
	//  int ptime = 0;
	int retval = 0;
	
	starttime = time(NULL);
	query_string = malloc(32000);
	
	if(!query_string) {
		fprintf(stderr, "! Failed to allocate memory. Fatal error, exiting.");
		retval = 20;
		goto clean_exit;
	}
	
	// Connect to mysql database
	con = mysql_init(NULL);
	if(con == NULL) {
		fprintf(stderr, "! Unable to initialize MySQL connection.\n");
		fprintf(stderr, "! Error message: %s\n", mysql_error(con));
		retval = 20;
		goto clean_exit;
	}
	
	if(mysql_real_connect(con, db_info->host, db_info->user, db_info->pass, 
			      NULL, 0, NULL, 0) == NULL) {
		fprintf(stderr, "! %s\n", mysql_error(con));
		retval = 20;
		goto clean_exit;
	}
	
	// Select the database to use
	sprintf(query_string, "USE %s;", db_info->name);
	if(mysql_query(con, query_string)) {
		fprintf(stderr, "! Database %s not found, or not possible to connect. \n", 
			db_info->name);
		printf("! %s\n", mysql_error(con));
		retval = 20;
		goto clean_exit;
	}
	
	// Open log file and prepare for data
	logfile = fopen(filename, "r");
	if ( logfile == NULL ) {
		fprintf(stderr, "! Error, unable to open syslog file %s.\n", filename);
		retval = 20;
		goto clean_exit;
	}
  
	lines = 0;
	row_counter = 0;
	
	while (( read = getline(&line, &len, logfile)) != -1) {
		lines++;
		if(strstr(line, prefix)) {

			// This entire part here is in order to future be able to add fields for time stamps to
			// the database and thereby be able to analyze happenings over time or find some times
			// more frequent than others and so on.

			/*
			month_string = strtok(line, " ");
			day_string = strtok(NULL, " ");
			hour_string = strtok(NULL, ":");
			minute_string = strtok(NULL, ":");
			second_string = strtok(NULL, " ");
			
			// Create a ISO date
			for ( i = 0; i < 12; i++) {
				if (!strcmp(month_string, months[i]))
					break;
			}
			
			month = i + 1;
			errno = 0;
			
			day = strtol(day_string, &dummyptr, 10);
			hour = strtol(hour_string, &dummyptr, 10);
			minute = strtol(minute_string, &dummyptr, 10);
			second = strtol(second_string, &dummyptr, 10);
			if(errno != 0) {
				fprintf(stderr, "! Error parsing day.\n");
				retval = 20;
				goto clean_exit;
			}
			
			host = strtok(NULL, " ");
			module = strtok(NULL, ": [");
			host_time = strtok(NULL, "] ");
			log_prefix = strtok(NULL, " "); 
			*/
			
			// Wind the "tape" until "IPT:" is found
			module = strtok(line, " ");
			do {
				module = strtok(NULL, " ");
			} while (strcmp(module, "IPT:"));
			
			// Next one will be our action here
			log_action = strtok(NULL, " ");
			
			// Clear records for next run
			log_ifin = log_ifout = log_mac = log_src = log_dst = 
				log_proto = log_src_port = log_dst_port = nullstring;
			
			
			// In here we process the known prefixes and
			// the data * from each field in the log
                        // string. Fields that are * not known are
                        // just ignored silently. Anything that is
                        // changed or added here needs to reflect the database
			
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
					continue;
				}
				if(!strncmp("DST=", token, 4)) {
					log_dst = token + strlen("DST=");
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
			
			// If row counter is 0 then we should prepare the insert string
			// with the headers needed.
			if(row_counter == 0) {
				sprintf(query_string, "INSERT INTO %s ( if_in, if_out, src_ip, src_prt, dst_ip, " \
					"dst_prt, proto, action, mac) VALUES ",
					db_info->table);
			} else {
				// Not the first line, then add the comma to previous line and
				// prepp for adding a new line.
				sprintf(query_string, "%s,\n", query_string);
			}
			
			// This adds the values to the query string
			sprintf(query_string, 
				"%s\n ( '%s', '%s', INET_ATON('%s'), '%s', INET_ATON('%s'), " \
				"'%s', '%s', '%s', '%s' )", 
				query_string, log_ifin, log_ifout, log_src, log_src_port, log_dst, log_dst_port, 
				log_proto, log_action, log_mac);
			row_counter++;
			
			// Every 100 lines we terminate the query string and then call
			// the MySQL to insert the rows collected. When done we must
			// reset the row_counter to 0 again.
			
			if(row_counter == QUERY_ROW_COUNT) {
				sprintf(query_string, "%s;", query_string);
				fprintf(stderr, "- Processed %d lines in %d seconds, %d bytes in query  \r", 
					lines, (int)time(NULL)-(int)starttime, (int)strlen(query_string) );
				
				if(mysql_query(con, query_string)) {
					printf("\n%s\n", mysql_error(con));
					retval = RETVAL_ERROR;
					goto clean_exit;
				}
				
				row_counter = 0;
			}
		}
		free(line);
		line = NULL;
	}
	
	// insert any remaining rows not previously inserted
	if(row_counter != 0) {
		sprintf(query_string, "%s;", query_string);
		if(mysql_query(con, query_string)) {
			fprintf(stderr, "%s\n", mysql_error(con));
			retval = RETVAL_ERROR;
			goto clean_exit;
		}
	}    
	
	sprintf(query_string, "COMMIT;");
	if(mysql_query(con, query_string)) {
		fprintf(stderr, "%s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
	
	//  worktime = time(NULL) - starttime;
	
	fprintf(stderr, "* Processed %d lines in %d seconds\n", 
		lines, (int)time(NULL)-(int)starttime);
	
	fprintf(stderr, "\n* Done processing file. %d records inserted in database.\n", lines);
	
	// Make sure everything is returned nicely after allocation by
        // us or by some procedure that we are calling
	
clean_exit:
	
	free(line);
	free(query_string);
	fcloseall();
	mysql_close(con);
	
	return retval;
}
