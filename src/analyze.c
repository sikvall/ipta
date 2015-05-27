/**********************************************************************
 * analyze.c
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

/* This is a dummy comment. The file has been reverted to an older
   version because of messing it up. This is just added so we can
   check it in again under a new version number. */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <my_global.h>
#include <mysql.h>
#include "ipta.h"

int analyze(struct ipta_db_info *db_info, struct ipta_flags *flags, int analyze_limit) 
{
	char *query = NULL;
	MYSQL *con = NULL;
	MYSQL_RES *result = NULL;
	MYSQL_ROW row = 0;
        //  int num_fields = 0;
	//  int i = 0;
	char src_ip_hostname[HOSTNAME_MAX_LEN];
	char dst_ip_hostname[HOSTNAME_MAX_LEN];
	int rdns_flg[2];
	int retval = RETVAL_OK;
	
	/* Allocate memory for the query string */
	query = malloc(QUERY_STRING_SIZE);
	if(!query) {
		printf("! Memory allocation failed.\n");
		retval = RETVAL_ERROR; 
		goto clean_exit;
	}
	
	// Connect to mysql database
	con = mysql_init(NULL);
	if(con == NULL) {
		printf("! Unable to initialize MySQL connection.\n");
		printf("! Error message: %s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
  
	if(mysql_real_connect(con, 
			      db_info->host, 
			      db_info->user, 
			      db_info->pass, 
			      NULL, 0, NULL, 0) == NULL) {
		printf("! %s\n", mysql_error(con));
		mysql_close(con);
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
  
	/* Select the database to use */
	sprintf(query, "USE %s;", db_info->name);
	if(mysql_query(con, query)) {
		printf("! Database %s not found, or not possible to connect. \n", db_info->name);
		printf("! %s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}

	/* This is a query that will show top 10 ranked IP addresses and
	 * their destination ports and protocol used to attack. All "ALLOW"
	 * traffic is sorted out 
	 */
	
	sprintf(query, 
		"SELECT COUNT(*), INET_NTOA(src_ip), src_prt, INET_NTOA(dst_ip), dst_prt, proto, " \
		"action FROM %s WHERE action<>'ACCEPT' AND if_in<>'lo' and if_out<>'lo' GROUP BY " \
		"src_ip, dst_prt, action, proto ORDER BY COUNT(*) DESC LIMIT %d;", 
		db_info->table, analyze_limit);
	
	if(mysql_query(con, query)) {
		printf("! Query not accepted from database.\n");
		printf("! %s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
	result = mysql_store_result(con);

	/* Present each row in the query that was returned to us in a readable
	 * fashtion. Substituting null fields with "-" */
	printf("\nShowing denied traffic grouped by IP, destination port, action taken and protocol.\n");
	printf(" Count Source IP                 SPort Dest IP                   DPort Proto  Action\n");
	printf("------ ------------------------- ----- ------------------------- ----- ------ ----------\n");
	rdns_flg[0] = rdns_flg[1] = FLAG_CLEAR;
	while((row = mysql_fetch_row(result))) {
		if(flags->rdns) {
			rdns_flg[0] = rdns_flg[1] = FLAG_SET;
			if(get_host_by_addr(row[1], src_ip_hostname, 25))
				rdns_flg[0] = FLAG_CLEAR;
			if(get_host_by_addr(row[3], dst_ip_hostname, 25))
				rdns_flg[1] = FLAG_CLEAR;
		}      
		printf("%6d %-25s %5d %-25s %5d %-6s %-10s\n", 
		       atoi(row[0]), rdns_flg[0] ? src_ip_hostname : row[1], 
		       atoi(row[2]), rdns_flg[1] ? dst_ip_hostname : row[3], 
		       atoi(row[4]), row[5], row[6]);
	}
	mysql_free_result(result);
	result = NULL;
	
	
	/* Create query to run */
	sprintf(query, 
		"SELECT COUNT(*), INET_NTOA(src_ip), INET_NTOA(dst_ip), action FROM %s WHERE proto='ICMP' " \
		"AND if_in<>'lo' AND if_out<>'lo' GROUP BY src_ip, dst_prt, action, proto ORDER BY COUNT(*) " \
		"DESC LIMIT %d;", 
		db_info->table, analyze_limit);
	
	if(mysql_query(con, query)) {
		printf("! Query not accepted from database.\n");
		printf("! %s\n", mysql_error(con));
		return RETVAL_ERROR;
	}
	result = mysql_store_result(con);

	/* Present each row in the query that was returned to us in a readable
	   fashtion. Substituting null fields with "-" */
	printf("\nShowing ICMP traffic statistics\n");
	printf(" Count Source IP                 Dest IP                   Action    \n");
	printf("------ ------------------------- ------------------------- ----------\n");
	rdns_flg[0] = rdns_flg[1] = FLAG_CLEAR;
	while((row = mysql_fetch_row(result))) {
		if(flags->rdns) {
			rdns_flg[0] = rdns_flg[1] = FLAG_SET;
			if(get_host_by_addr(row[1], src_ip_hostname, 25))
				rdns_flg[0] = FLAG_CLEAR;
			if(get_host_by_addr(row[2], dst_ip_hostname, 25))
				rdns_flg[1] = FLAG_CLEAR;
		}      
		
		printf("%6d %-25s %-25s %-10s\n", 
		       atoi(row[0]), rdns_flg[0] ? src_ip_hostname : row[1], 
		       rdns_flg[1] ? dst_ip_hostname : row[2], row[3]);
	}
	mysql_free_result(result);
	result = NULL;
	
	/* Create query to run */
	sprintf(query, 
		"SELECT COUNT(*), dst_prt, proto, action FROM %s WHERE if_in<>'lo' AND if_out<>'lo' " \
		"AND action<>'ACCEPT' GROUP BY dst_prt, action, proto ORDER BY COUNT(*) DESC LIMIT %d;", 
		db_info->table, analyze_limit);
	
	if(mysql_query(con, query)) {
		printf("! Query not accepted from database.\n");
		printf("! %s\n", mysql_error(con));
		return RETVAL_ERROR;
	}
	result = mysql_store_result(con);

	/* Present each row in the query that was returned to us in a readable
	   fashtion. Substituting null fields with "-" */
	printf("\nMost denied ports\n");
	printf(" Count   DPort   Proto    Action       \n");
	printf("------   -----   ------   ----------   \n");
	while((row = mysql_fetch_row(result))) {
		printf("%6d    %5d   %-6s   %-10s\n", 
		       atoi(row[0]), atoi(row[1]), row[2], row[3]);
	}
	
	mysql_free_result(result);
	result = NULL;
	
	
	
	// Create query to run
	sprintf(query, 
		"SELECT COUNT(*), INET_NTOA(src_ip), src_prt, INET_NTOA(dst_ip), dst_prt, proto FROM %s " \
		"WHERE if_in<>'lo' AND if_out<>'lo' AND action='INVALID' GROUP BY src_ip, dst_prt, proto " \
		"ORDER BY COUNT(*) DESC LIMIT %d;", 
		db_info->table, analyze_limit);
	
	if(mysql_query(con, query)) {
		printf("! Query not accepted from database.\n");
		printf("! %s\n", mysql_error(con));
		return RETVAL_ERROR;
	}
	result = mysql_store_result(con);
	
	/* Present each row in the query that was returned to us in a readable
	   fashtion. Substituting null fields with "-" */
	printf("\nMost invalid packets comes from\n");
	printf(" Count   Source IP                   SPort   Dest IP                     DPort   Proto    \n");
	printf("------   -------------------------   -----   -------------------------   -----   ------   \n");
	
	rdns_flg[0] = rdns_flg[1] = FLAG_CLEAR;
	while((row = mysql_fetch_row(result))) {
		if(flags->rdns) {
			rdns_flg[0] = rdns_flg[1] = FLAG_SET;
			if(get_host_by_addr(row[1], src_ip_hostname, 25))
				rdns_flg[0] = FLAG_CLEAR;
			if(get_host_by_addr(row[3], dst_ip_hostname, 25))
				rdns_flg[1] = FLAG_CLEAR;
			
		}
		printf("%6d   %-25s   %5d   %-25s   %5d   %-6s   \n", 
		       atoi(row[0]), rdns_flg[0] ? src_ip_hostname : row[1], atoi(row[2]), 
		       rdns_flg[1] ? dst_ip_hostname : row[3], atoi(row[4]), row[5]);
	}
	mysql_free_result(result);
	result = NULL;
	
        /* Create query to run */
	sprintf(query, 
		"SELECT COUNT(*),if_in,action,proto FROM %s WHERE action<>'ACCEPT' GROUP BY " \
		"if_in,action,proto ORDER BY COUNT(*) DESC LIMIT %d;", 
		db_info->table, analyze_limit);
	
	if(mysql_query(con, query)) {
		printf("! Query not accepted from database.\n");
		printf("! %s\n", mysql_error(con));
		return RETVAL_ERROR;
	}
	result = mysql_store_result(con);
	
	/* Present each row in the query that was returned to us
	   in a readable fashtion. Substituting null fields with "-" */
	printf("\nInterface statistics\n");
	printf(" Count   IF In        Action       Proto\n");
	printf("------   ----------   ----------   -----\n");
	while((row = mysql_fetch_row(result))) {
		printf("%6d   %-10s   %-10s   %-6s   \n", 
		       atoi(row[0]), row[1], row[2], row[3]);
	}
	mysql_free_result(result);
	result = NULL;
	
clean_exit:

	free(query);
	if(result)
		mysql_free_result(result);
	if(con)
		mysql_close(con);

	return retval;
}
