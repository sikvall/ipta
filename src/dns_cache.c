/**********************************************************************
 * dns_cache.c
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
#include <my_global.h>
#include <mysql.h>
#include "ipta.h"


/***********************************************************************
 * dns_open_db
 *
 * DESCRIPTION
 *
 * This is a simple helper function that will open the con, select the
 * right database from the parameters given in the ipta_db_info
 * struct. Upon success it will return a MYSQL *con object. In failure
 * it will just return NULL.
 * 
 * PARAMETERS 
 *
 * struct ipta_db_info *db - a pointer to a struct containing
 *                           necessary information.
 *
 * RETURNS
 *
 * Upon Success: A MYSQL *con object pointer.
 *
 * Upon Failure: NULL
 *
 * OBSERVE! 
 *
 * The called must take care to destroy the con object themselves when
 * done with it because this function will do nothing of the kind.
 ***********************************************************************/
MYSQL *dns_open_db(struct ipta_db_info *db) 
{ 
	MYSQL *con = NULL; 
	int retval = RETVAL_OK; 
	char query_string[256]; // Fixme
	
	con = mysql_init(NULL);
	if(con == NULL) {
		fprintf(stderr, "! Error, unable to initiate MySQL.\n");
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
	
	/* Attempt proper connection to database */
	if(mysql_real_connect(con, db->host, db->user, db->pass, 
			      NULL, 0, NULL, 0) == NULL) {
		fprintf(stderr, "! Unable to connect to database.\n");
		fprintf(stderr, "  Error: %s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
	
	/* Select the indicated database */
	sprintf(query_string, "USE %s;", db->name);
	if(mysql_query(con, query_string)) {
		fprintf(stderr, "! Database %s not found, or not possible to connect.\n", db->name);
		fprintf(stderr, "  Error: %s\n", mysql_error(con));
		goto clean_exit;
	}

	/* This is where we would end up if all is ok, we should NOT destroy
	 * con in this case. */
	return con;
	
clean_exit:
	if(retval != RETVAL_OK) {
		if(con) 
			mysql_close(con);
	}
	return NULL;
}


/*****************************************************************************
 * dns_cache_create_table creates a new table and populates it with
 * the necessary parths to be used later as a dns_cache for the ipta
 * system
 *****************************************************************************/
int dns_cache_create_table(struct ipta_db_info *db) 
{
	char *query_string = NULL;
	MYSQL *con = NULL;
	int retval = RETVAL_OK;
	
	/* Allocate memory */
	query_string = calloc(1, 10000);
	if(NULL == query_string) {
		fprintf(stderr, "! Error, unable to allocate memory. Quitting!\n");
		exit(RETVAL_ERROR);
	}
	
	con = dns_open_db(db);
	if(!con) {
		fprintf(stderr, "! Error, unable to open database.\n");
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
	
	/* Create the query needed to create the database table */
	sprintf(query_string, 
		"CREATE TABLE %s ("		      \
		"ip INT(10) UNSIGNED PRIMARY KEY NOT NULL,"	\
		"host VARCHAR(256) DEFAULT NULL,"		\
		"ttl TIMESTAMP);",
		db->table);
	
	/* Attempt to create the table */
	if(mysql_query(con, query_string)) {
		fprintf(stderr, 
			"! Unable to create table.\n"	\
			"  Error: %s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
	
	fprintf(stderr, "+ Table %s created in database %s.\n",
		db->table, db->name);

	/* Table is created, clean up and exit nicely */
	retval = RETVAL_OK;
	
	/* Return resources, clean up and return to caller with retval */
clean_exit:
	free(query_string);
	if(con) 
		mysql_close(con);

	return retval;
}


/***********************************************************************
 * dns_cache_add
 *
 * Adds a record to the ipta DNS cache system.
 *
 ***********************************************************************/
int dns_cache_add(struct ipta_db_info *db, char *ip_address, char *hostname) 
{
	char *query_string = NULL;
	MYSQL *con = NULL;
	int retval = 0;
	
	query_string = calloc(1,10000); // Fix this later
	if(!query_string) {
		fprintf(stderr, "! Unable to allocate memory!\n");
		assert(0);
	}
	
	/* Initialize databse object */
	con = dns_open_db(db);
	if(con == NULL) {
		printf("! Unable to initialize MySQL connection.\n");
		printf("  Error message: %s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
	
	/* Attempt to update the key */
	sprintf(query_string, 
		"REPLACE INTO %s (ip, host, ttl) VALUES ("	\
		"INET_ATON('%s'), '%s', now());",
		db->table, ip_address, hostname);
	
	if(mysql_query(con, query_string)) {
		fprintf(stderr, 
			"! Unable to perform insertion in to table.\n"	\
			"  Error: %s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
	
clean_exit:
	free(query_string);
	if(con)
		mysql_close(con);
	return retval;
}





/***********************************************************************
 * dns_cache_get
 *
 * DESCRIPTION
 *
 * Takes a db_info object, a pointer to a string containing an IP
 * address on "dotted format notation", a pointer to a hostname which
 * will be overwritten with the name if found and a ttl as a string
 * containing the maximum number of hours old the cache value is okay
 * to use.
 *
 * The function returns RETVAL_OK if successful and writes the found
 * hostname to the pointer passed to it. If unsuccessful it will
 * return RETVAL_ERROR and the hostname written will be the empty
 * string.
 */
int dns_cache_get(struct ipta_db_info *db, char *ip_address, char *hostname, char *ttl) 
{
	char *query_string = NULL;
	MYSQL *con = NULL;
	int retval = RETVAL_OK;
	MYSQL_RES *result = NULL;
	MYSQL_ROW row = 0;
	//int num_fields = 0;
	
	/* Allocate memory */
	query_string = malloc(10000); // Fix later
	if(!query_string) {
		fprintf(stderr, "! Allocation failed!\n");
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
	
	/* Initialize databse object */
	con = dns_open_db(db);
	if(con == NULL) {
		printf("! Unable to initialize MySQL connection.\n");
		printf("  Error message: %s\n", mysql_error(con));
		retval = 20;
		goto clean_exit;
	}
	
	/* Attempt to query database */
	sprintf(query_string,
		"SELECT host FROM %s "			\
		"WHERE ip=INET_ATON('%s') "		\
		"AND ttl > NOW() - INTERVAL '%s' HOUR;",
		db->table, ip_address, ttl);
	if(mysql_query(con, query_string)) {
		fprintf(stderr, 
			"! Querying database failed.\n"
			"  Error: %s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
	
	result = mysql_store_result(con);
//	num_fields = mysql_num_fields(result);
	row = mysql_fetch_row(result);
	if(row)  {
		strcpy(hostname, row[0]);
		retval = RETVAL_OK;
	} else{
		strcpy(hostname, "");
		retval = RETVAL_WARN;
	}
	
clean_exit:
	if(query_string)
		free(query_string);
	if(result)
		mysql_free_result(result);
	if(con)
		mysql_close(con);
	
	return retval;
	
}


/***********************************************************************
 * Drop any record that has an expired TTL in the database when this
 * function is called. The TTL is set in form of a date. When new
 * records are added they are added with a default TTL determined in
 * ipta.h. Expiry will look up todays date and select all records that
 * have older dates and then remove them.
 *
 * PARAMETERS
 * 
 * struct ipta_db_info *db
 *     Pointer to struct ipta_db_info detailing the database
 *     connection and other parameters needed to identify the table.
 *     Defined in ipta.h
 *
 * RETURNS
 *
 * int : The number of selected/deleted rows that was expired from the
 *       database.
 *
 ***********************************************************************/
int dns_cache_prune(struct ipta_db_info *db) { 
  fprintf(stderr, "Function is a stub and not implemented.\n");
  assert(0);

  return RETVAL_ERROR;
}

/**********************************************************************
 * Delete a table pointed out by the struct and return success
 * RETVAL_OK if all went well or error code if there was a problem.
 **********************************************************************/
int dns_cache_delete_table(struct ipta_db_info *db) 
{
	MYSQL *con = NULL;
	char *query = NULL;
	int retval = RETVAL_OK;
	
	query = malloc(10000);
	if(!query) {
		fprintf(stderr, "! Error, memory allocation failed.\n");
		retval = RETVAL_ERROR;
		goto clean_exit;
	}

	con = dns_open_db(db);
	if(!con) {
		fprintf(stderr, "! Error, unable to initialize Mysql.\n");
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
	
clean_exit:
	free(query);
	if(con)
		mysql_close(con);
	return retval;
}

/***********************************************************************
 * clear the entire table which removes all records but lets the table
 * remain to be populated by ne records.
 ***********************************************************************/
int dns_cache_clear_table(struct ipta_db_info *db) 
{
	fprintf(stderr,"Function is a stub and not implemented.\n");
	assert(0);
	return RETVAL_ERROR;
}
