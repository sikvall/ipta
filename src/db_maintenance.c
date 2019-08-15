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
#include <sys/stat.h>
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
MYSQL *open_db(struct ipta_db_info *db) 
{ 
	MYSQL *con = NULL; 
	int retval = RETVAL_OK; 
	char query_string[QUERY_STRING_SIZE];
	
	con = mysql_init(NULL);
	if(con == NULL) {
		fprintf(stderr, "! Error, unable to initiate MySQL.\n");
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
	
	// Attempt proper connection to database
	if(mysql_real_connect(con, db->host, db->user, db->pass, 
			      NULL, 0, NULL, 0) == NULL) {
		fprintf(stderr, "! Unable to connect to database.\n");
		fprintf(stderr, "  Error: %s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
	
	// Select the indicated database
	sprintf(query_string, "USE %s;", db->name);
	if(mysql_query(con, query_string)) {
		fprintf(stderr, "! Database %s not found, or not possible to connect.\n", db->name);
		fprintf(stderr, "  Error: %s\n", mysql_error(con));
		goto clean_exit;
	}

	// If all is OK we end up here, NOT destroying the con object
	return con;
	
clean_exit:
	if(retval != RETVAL_OK) {
		if(con) 
			mysql_close(con);
	}
	return NULL;
}

int create_db(struct ipta_db_info *db)
{
	MYSQL *con = NULL;
	char *query = NULL;
	int retval = RETVAL_OK;


	// TODO: There is a built-in bug here when using the open_db()
	// function since we don't have the database yet but this
	// function assumes it! We must replace it with our own
	// version in this case. But that is a job for tomorrow. 
	con = open_db(db);
	if(!con) {
		fprintf(stderr, "! Failed to open the database, exiting.\n");
		retval = RETVAL_ERROR;
		goto clean_exit;
	}

	query = malloc(10000); // fixme
	if(!query) {
		fprintf(stderr, "! Allocation failed, must exit.\n");
		retval = RETVAL_ERROR;
		goto clean_exit;
	}

	sprintf(query, "CREATE DATABASE %s;", db->name);
	if(mysql_query(con, query)) {
		fprintf(stderr, "! Error, unable to create database '%s'.\n"
			"  Error: %s.\n", db->name, mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
	
	sprintf(query, "GRANT ALL PRIVILEGES ON %s.* TO '%s'@'localhost' IDENTIFIED BY %s;",
		db->name, db->user, db->pass);
	if(mysql_query(con, query)) {
		fprintf(stderr, "! Error, unable to grand privileges.\n"
			"  Error: %s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}

clean_exit:
	if(con)
		mysql_close(con);
	free(query);

	return retval;
}



/***********************************************************************
 * create_table
 *
 * The function create_table creates a new table in the existing
 * database with the correct number of columns and their definitions
 * to be used by ipta
 ***********************************************************************/
int create_table(struct ipta_db_info *db)
{
	char *query = NULL;
	MYSQL *con = NULL;
	int retval = RETVAL_OK;

	con = open_db(db);
	if(!con) {
		fprintf(stderr, "! Unable to open database connection, giving up!\n");
		retval = RETVAL_ERROR;
		goto clean_exit;
	}

	// Just grab some heap
	query = malloc(10000);
	if(query == NULL) {
		fprintf(stderr, "! ERROR: Unable to allocate memory, exiting!\n");
		// Immediate exit
		exit(RETVAL_ERROR);
	}
	
	// Create the SQL to create the table.
	// There is no sanity check really for this, MySQL will have to do that for us.
	sprintf(query, 
		"CREATE TABLE %s ("					\
		"id int(11) PRIMARY KEY NOT NULL AUTO_INCREMENT,"	\
		"timestamp timestamp NOT NULL DEFAULT '1970-01-01 04:00:00'," \
		"if_in varchar(10) DEFAULT NULL,"			\
		"if_out varchar(10) DEFAULT NULL,"			\
		"src_ip int(10) unsigned DEFAULT NULL,"			\
		"src_prt int(10) unsigned DEFAULT NULL,"		\
		"dst_ip int(10) unsigned DEFAULT NULL,"			\
		"dst_prt int(10) unsigned DEFAULT NULL,"		\
		"proto varchar(10) DEFAULT NULL,"			\
		"action varchar(10) DEFAULT NULL,"			\
		"mac varchar(40) DEFAULT NULL);", 
		db->table);
	
	if(mysql_query(con, query)) {
		fprintf(stderr, "! Query not accepted from database.\n");
		fprintf(stderr, "! %s\n", mysql_error(con));
		
		// Set error condition and then clean_exit
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
	
	fprintf(stderr, "* Table '%s' created in database '%s' which you may now use.\n", 
		db->table, db->name);
	

	// Clear things up and exit with return value previously set
clean_exit:
	if(con)
		mysql_close(con);
	free(query);
	
	return retval;
}



/**********************************************************************
 * delete_table
 *
 * This function will perform a drop table on the selected table. This
 * is different from the clear_table function in that the table do not
 * remain after running this. When this is done you may need to create
 * a new table in order to use ipta again.
 *********************************************************************/
int delete_table(struct ipta_db_info *db)
{
	MYSQL *con = NULL;
	char *query = NULL;
	int retval = RETVAL_OK;
	
	// Connect to mysql database
	con = open_db(db);
	if(con == NULL) {
		fprintf(stderr, "! ERROR: Unable to initialize MySQL connection.\n");
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
	
	query = calloc(10000,1);
	if(!query) {
		fprintf(stderr, "! Error: Failed allocation, exit now.\n");
		exit(RETVAL_ERROR);
	}
	
	// Empty the table before populating it with new data if flag set
	sprintf(query, "DROP TABLE %s", db->table);
	if(mysql_query(con, query)) {
		fprintf(stderr, "%s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
	
	fprintf(stderr,"* Table %s deleted from database %s.\n",
		db->table, db->name);
	
clean_exit:
	free(query);
	if(con)
		mysql_close(con);
	return retval;
}



/**********************************************************************
 * list_tables
 *
 * This function will just list the current tables in the database for
 * the user so he can check what tables he has been using. It will
 * also show the number of records in each database so it is possible
 * to detect any databases that are currently not in use.
 *********************************************************************/
int list_tables(struct ipta_db_info *db_info)
{
	MYSQL *con = NULL;
	char *query = NULL;
	int retval = RETVAL_OK;
	MYSQL_ROW row;
	MYSQL_RES *result = NULL;
	int i = 0;
	int num_fields = 0;
	int row_counter = 0;

	// Connect to mysql database
	con = open_db(db_info);
	if(con == NULL) {
		fprintf(stderr, "ERROR: Unable to initialize MySQL connection.\n");
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
  
	query = calloc(QUERY_STRING_SIZE, 1);
	if(!query) {
		fprintf(stderr, "! Error, allocation failed, immediate exit.\n");
		exit(RETVAL_ERROR);
	}

	// Empty the table before populating it with new data if flag set
	sprintf(query, "SHOW TABLES");
	if(mysql_query(con, query)) {
		fprintf(stderr, "%s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
  
	result = mysql_store_result(con);
	num_fields = mysql_num_fields(result);

	while ((row = mysql_fetch_row(result))) { 
		row_counter++;
		for(i = 0; i < num_fields; i++) { 
			printf("  %3d: %s ", row_counter, row[i] ? row[i] : "NULL"); 
		} 
		printf("\n"); 
	}

clean_exit:
	mysql_free_result(result);
	free(query);
	if(con)
		mysql_close(con);
	
	return retval;
}



/**********************************************************************
 * clear_database
 *
 * This function clears the database from all data, it does not drop
 * the database, just delete the already populated data in it in order
 * to make it ready to receive new data.
 *********************************************************************/
int clear_database(struct ipta_db_info *db_info)
{
	MYSQL *con = NULL;
	char *query = NULL;
	int retval = 0;

	query = calloc(QUERY_STRING_SIZE, 1);
	if(!query) {
		fprintf(stderr, "! Error, failed allocation of memory.\n");
		retval = RETVAL_ERROR;
		goto clean_exit;
	}

	// Connect to mysql database
	con = open_db(db_info);
	if(con == NULL) {
		fprintf(stderr, "ERROR: Unable to initialize MySQL connection.\n");
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
  
	// Empty the table before populating it with new data if flag set
	sprintf(query, "DELETE FROM %s;", db_info->table);
	if(mysql_query(con, query)) {
		fprintf(stderr, "%s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}

clean_exit:
	if(NULL != con)
		mysql_close(con);

	return retval;
}
