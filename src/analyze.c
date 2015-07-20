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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <my_global.h>
#include <mysql.h>
#include "ipta.h"

int analyze(struct ipta_db_info *db, struct ipta_flags *flags, int analyze_limit, struct ipta_db_info *dnsdb) 
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
	
	// Allocate memory for the query string
	query = malloc(QUERY_STRING_SIZE);
	if(!query) {
		fprintf(stderr, "! Memory allocation failed.\n");
		retval = RETVAL_ERROR; 
		goto clean_exit;
	}

	// Open the con to process queries
	con = open_db(db);
	if(!con) {
		fprintf(stderr, "! Unable to initialize MySQL connection.\n");
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
  
	// Query: Top culprits ordered by source ip, destination port, action taken and protocol
	sprintf(query, 
		"SELECT COUNT(*), INET_NTOA(src_ip), src_prt, INET_NTOA(dst_ip), dst_prt, proto, " \
		"action FROM %s WHERE action<>'ACCEPT' AND if_in<>'lo' and if_out<>'lo' GROUP BY " \
		"src_ip, dst_prt, action, proto ORDER BY COUNT(*) DESC LIMIT %d;", 
		db->table, analyze_limit);
	
	if(mysql_query(con, query)) {
		fprintf(stderr, "! Query not accepted from database.\n");
		fprintf(stderr, "  %s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
	result = mysql_store_result(con);

	printf("\nShowing denied traffic grouped by IP, destination port, action taken and protocol.\n");
	printf(" Count Source IP                 SPort Dest IP                   DPort Proto  Action\n");
	printf("------ ------------------------- ----- ------------------------- ----- ------ ----------\n");
	rdns_flg[0] = rdns_flg[1] = FLAG_CLEAR;
	while((row = mysql_fetch_row(result))) {
		if(flags->rdns) {
			rdns_flg[0] = rdns_flg[1] = FLAG_SET;
			if(get_host_by_addr(row[1], src_ip_hostname, 25, dnsdb))
				rdns_flg[0] = FLAG_CLEAR;
			if(get_host_by_addr(row[3], dst_ip_hostname, 25, dnsdb))
				rdns_flg[1] = FLAG_CLEAR;
		}      
		printf("%6d %-25s %5d %-25s %5d %-6s %-10s\n", 
		       atoi(row[0]), rdns_flg[0] ? src_ip_hostname : row[1], // rdns flag determines host or ip
		       atoi(row[2]), rdns_flg[1] ? dst_ip_hostname : row[3], 
		       atoi(row[4]), row[5], row[6]);
	}
	mysql_free_result(result);
	result = NULL;
	
	// Create a query for ICMP protocol use
	sprintf(query, 
		"SELECT COUNT(*), INET_NTOA(src_ip), INET_NTOA(dst_ip), action FROM %s WHERE proto='ICMP' " \
		"AND if_in<>'lo' AND if_out<>'lo' GROUP BY src_ip, dst_prt, action, proto ORDER BY COUNT(*) " \
		"DESC LIMIT %d;", 
		db->table, analyze_limit);
	
	if(mysql_query(con, query)) {
		fprintf(stderr, "! Query not accepted from database.\n");
		fprintf(stderr, "! %s\n", mysql_error(con));
		return RETVAL_ERROR;
	}
	result = mysql_store_result(con);

	printf("\nShowing ICMP traffic statistics\n");
	printf(" Count Source IP                 Dest IP                   Action    \n");
	printf("------ ------------------------- ------------------------- ----------\n");
	rdns_flg[0] = rdns_flg[1] = FLAG_CLEAR;
	while((row = mysql_fetch_row(result))) {
		if(flags->rdns) {
			rdns_flg[0] = rdns_flg[1] = FLAG_SET;
			if(get_host_by_addr(row[1], src_ip_hostname, 25, dnsdb))
				rdns_flg[0] = FLAG_CLEAR;
			if(get_host_by_addr(row[2], dst_ip_hostname, 25, dnsdb))
				rdns_flg[1] = FLAG_CLEAR;
		}      
		
		printf("%6d %-25s %-25s %-10s\n", 
		       atoi(row[0]), rdns_flg[0] ? src_ip_hostname : row[1], 
		                     rdns_flg[1] ? dst_ip_hostname : row[2], row[3]);
	}
	mysql_free_result(result);
	result = NULL;
	
	// Query: Not accepted packets ordered by destination port, action, protocol
	sprintf(query, 
		"SELECT COUNT(*), dst_prt, proto, action FROM %s WHERE if_in<>'lo' AND if_out<>'lo' " \
		"AND action<>'ACCEPT' GROUP BY dst_prt, action, proto ORDER BY COUNT(*) DESC LIMIT %d;", 
		db->table, analyze_limit);
	
	if(mysql_query(con, query)) {
		fprintf(stderr, "! Query not accepted from database.\n");
		fprintf(stderr, "! %s\n", mysql_error(con));
		return RETVAL_ERROR;
	}
	result = mysql_store_result(con);

	printf("\nMost denied ports\n");
	printf(" Count   DPort   Proto    Action       \n");
	printf("------   -----   ------   ----------   \n");
	while((row = mysql_fetch_row(result))) {
		printf("%6d    %5d   %-6s   %-10s\n", 
		       atoi(row[0]), atoi(row[1]), row[2], row[3]);
	}
	
	mysql_free_result(result);
	result = NULL;
	
	// Query: Shows invalid packets ordered by src ip, destination port, and protocol.
	sprintf(query, 
		"SELECT COUNT(*), INET_NTOA(src_ip), src_prt, INET_NTOA(dst_ip), dst_prt, proto FROM %s " \
		"WHERE if_in<>'lo' AND if_out<>'lo' AND action='INVALID' GROUP BY src_ip, dst_prt, proto " \
		"ORDER BY COUNT(*) DESC LIMIT %d;", 
		db->table, analyze_limit);
	
	if(mysql_query(con, query)) {
		fprintf(stderr, "! Query not accepted from database.\n");
		fprintf(stderr, "! %s\n", mysql_error(con));
		return RETVAL_ERROR;
	}
	result = mysql_store_result(con);
	
	printf("\nMost invalid packets comes from\n");
	printf(" Count   Source IP                   SPort   Dest IP                     DPort   Proto    \n");
	printf("------   -------------------------   -----   -------------------------   -----   ------   \n");
	
	rdns_flg[0] = rdns_flg[1] = FLAG_CLEAR;
	while((row = mysql_fetch_row(result))) {
		if(flags->rdns) {
			rdns_flg[0] = rdns_flg[1] = FLAG_SET;
			if(get_host_by_addr(row[1], src_ip_hostname, 25, dnsdb))
				rdns_flg[0] = FLAG_CLEAR;
			if(get_host_by_addr(row[3], dst_ip_hostname, 25, dnsdb))
				rdns_flg[1] = FLAG_CLEAR;
			
		}
		printf("%6d   %-25s   %5d   %-25s   %5d   %-6s   \n", 
		       atoi(row[0]), rdns_flg[0] ? src_ip_hostname : row[1], atoi(row[2]), 
		       rdns_flg[1] ? dst_ip_hostname : row[3], atoi(row[4]), row[5]);
	}
	mysql_free_result(result);
	result = NULL;
	
	// Query: Not accepted packets ordered by interface, reason and protocol
	sprintf(query, 
		"SELECT COUNT(*),if_in,action,proto FROM %s WHERE action<>'ACCEPT' GROUP BY " \
		"if_in,action,proto ORDER BY COUNT(*) DESC LIMIT %d;", 
		db->table, analyze_limit);
	
	if(mysql_query(con, query)) {
		fprintf(stderr, "! Query not accepted from database.\n");
		fprintf(stderr, "! %s\n", mysql_error(con));
		return RETVAL_ERROR;
	}
	result = mysql_store_result(con);
	
	printf("\nInterface statistics\n");
	printf(" Count   IF In        Action       Proto\n");
	printf("------   ----------   ----------   -----\n");
	while((row = mysql_fetch_row(result))) {
		printf("%6d   %-10s   %-10s   %-6s   \n", 
		       atoi(row[0]), row[1], row[2], row[3]);
	}
	mysql_free_result(result);
	result = NULL;

	// Query: Destination ports with denied traffic and their actions
	sprintf(query,
		"SELECT COUNT(*), dst_prt, action FROM %s"\
		"WHERE if_in<>'lo' and if_in<>'' and action<>'ACCEPT' "\
		"GROUP BY dst_prt, action ORDER BY COUNT(*) DESC LIMIT %d;",
		db->table, analyze_limit);

	if(mysql_query(con, query)) {
		fprintf(stderr, "! Query not accepted from database.\n");
		fprintf(stderr, "! %s\n", mysql_error(con));
		return RETVAL_ERROR;
	}
	result = mysql_store_result(con);
	
	printf("\nInterface statistics\n");
	printf(" Count   DPort   Action\n");
	printf("------   -----   ----------\n");
	while((row = mysql_fetch_row(result))) {
		printf("%6d   %5d   %-10s   \n",
		       atoi(row[0]), atoi(row[1]), row[2]);
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
