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

/* >>>>> Todo: Description here */

int restore_db(struct ipta_db_info *db_info)
{
	FILE *fp;
	char *line;
	size_t len = 0;
	ssize_t read;
	
	fprintf(stderr, "* Warning, restore_db() is a stub function\n");
	
	/* Open the .ipta file */
	fp = fopen("~/.ipta", "r");
	if(NULL == fp) {
		fprintf(stderr, "! Error, unable to open .ipta file. Check permission and that it exists.\n");
		return RETVAL_ERROR;
	}
	
	/* File is open, read each line and find the keywords, then insert
	   into structure */
	while((read = getline(&line, &len, fp)) != -1) {
		if(!strncmp("host=", line, strlen("host="))) {
			/* Do stuff here */
			return RETVAL_OK;
		}
	}
  
	return RETVAL_OK;
}


/**********************************************************************
 * save_db
 *
 * This function saves a copy of the current db settings to a file
 * called .ipta that remains in the users home directory. This way we
 * can pick it up next time and restore the settings without having to
 * read them from the command line. 
 **********************************************************************/
int save_db(struct ipta_db_info *db_info)
{
	FILE *fp;
	int retval = RETVAL_OK;
	
	/* Open the file, or attempt to */
	fp = fopen("~/.ipta", "w");
	if(NULL == fp) {
		fprintf(stderr, "! Error, unable to open .ipta file! Check ownership and attributes.\n");
		return RETVAL_ERROR;
	}
	
	/* Enforce file persmissions and ownership as it contains passwords */
	retval = chmod("~/.ipta", 600);
	if(retval) {
		fprintf(stderr, "! Error, unable to set mode permissions, error code %d\n", retval);
		return RETVAL_ERROR;
	}
	
	/* File is open so output the data in a human readable fashion */
	fprintf(fp,				\
		"host=%s\n"			\
		"user=%s\n"			\
		"pass=%s\n"			\
		"name=%s\n"			\
		"table=%s",
		db_info->host,
		db_info->user,
		db_info->pass,
		db_info->name,
		db_info->table);
	
	/* Close file and return no error */
	fclose(fp);
	return RETVAL_OK;
	
}



/***********************************************************************
 * create_table
 *
 * The function create_table creates a new table in the existing
 * database with the correct number of columns and their definitions
 * to be used by ipta
 ***********************************************************************/
int create_table(struct ipta_db_info *db_info)
{
	char *query;
	MYSQL *con;
	int retval = RETVAL_OK;
	
	/* Just grab some heap */
	query = malloc(10000);
	if(query == NULL) {
		fprintf(stderr, "! ERROR: Unable to allocate memory, exiting!\n");
		/* Immediate exit */
		exit(RETVAL_ERROR);
	}
	
	/* Memory ok, initiate the con for MySQL */
	con = mysql_init(NULL);
	if(con == NULL) {
		fprintf(stderr, "! Unable to initialize MySQL connection.\n");
		fprintf(stderr, "! Error: %s\n", mysql_error(con));
		
		/* Set error condition and clean exit */
		retval = RETVAL_ERROR;
		goto finish;
	}
	
	/* Attempt a proper db connection */
	if(mysql_real_connect(con, 
			      db_info->host, 
			      db_info->user, 
			      db_info->pass, 
			      NULL, 0, NULL, 0) == NULL) {
		fprintf(stderr, "! %s\n", mysql_error(con));
		/* Set error condition and clean exit */
		retval = RETVAL_ERROR;
		goto finish;
	}
	
	/* Select the database */
	
	sprintf(query, "USE %s;", db_info->name);
	if(mysql_query(con, query)) {
		fprintf(stderr, "! Database %s not found, or not possible to connect. \n", db_info->name);
		fprintf(stderr, "! %s\n", mysql_error(con));
		/* Set error condition and clean exit */
		retval = RETVAL_ERROR;
		goto finish;
	}
	
	/* Create the SQL to create the table. No sanity check is performed
	   that the name is okay, that is something that MySQL will have to
	   take care for us at the moment. */
	
	sprintf(query, 
		"CREATE TABLE %s ("					\
		"id int(11) PRIMARY KEY NOT NULL AUTO_INCREMENT,"	\
		"timestamp timestamp NOT NULL DEFAULT '0000-00-00 00:00:00'," \
		"if_in varchar(10) DEFAULT NULL,"			\
		"if_out varchar(10) DEFAULT NULL,"			\
		"src_ip int(10) unsigned DEFAULT NULL,"			\
		"src_prt int(10) unsigned DEFAULT NULL,"		\
		"dst_ip int(10) unsigned DEFAULT NULL,"			\
		"dst_prt int(10) unsigned DEFAULT NULL,"		\
		"proto varchar(10) DEFAULT NULL,"			\
		"action varchar(10) DEFAULT NULL,"			\
		"mac varchar(40) DEFAULT NULL);", 
		db_info->table);
	
	if(mysql_query(con, query)) {
		fprintf(stderr, "! Query not accepted from database.\n");
		fprintf(stderr, "! %s\n", mysql_error(con));
		
		/* Set error condition and then clean_exit */
		retval = RETVAL_ERROR;
		goto finish;
	}
	
	fprintf(stderr, "* Table '%s' created in database '%s' which you may now use.\n", 
		db_info->table, db_info->name);
	

	/* Clear things up and exit with return value previously set */
finish:
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
int delete_table(struct ipta_db_info *db_info)
{
	MYSQL *con;
	char *query = NULL;
	int retval = RETVAL_OK;
	
	/* Connect to mysql database */
	fprintf(stderr, "* Opening database.\n");
	con = mysql_init(NULL);
	if(con == NULL) {
		fprintf(stderr, "! ERROR: Unable to initialize MySQL connection.\n");
		fprintf(stderr, "  Error message: %s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
	
	if(mysql_real_connect(con, 
			      db_info->host, 
			      db_info->user, 
			      db_info->pass, 
			      NULL, 0, NULL, 0) == NULL) {
		fprintf(stderr, "%s\n", mysql_error(con));
		retval = RETVAL_WARN;
		goto clean_exit;
	}
	
	query = calloc(10000,1);
	
	// Select the database to use
	sprintf(query, "USE %s;", db_info->name);
	if(mysql_query(con, query)) {
		fprintf(stderr, "%s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
	
	// Empty the table before populating it with new data if flag set
	sprintf(query, "DROP TABLE %s", db_info->table);
	if(mysql_query(con, query)) {
		fprintf(stderr, "%s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
	
	fprintf(stderr,"* Table %s deleted from database %s.\n",
		db_info->table, db_info->name);
	
clean_exit:
	free(query);
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
	MYSQL *con;
	char *query = NULL;
	int retval = RETVAL_OK;
	MYSQL_ROW row;
	MYSQL_RES *result = NULL;
	int i = 0;
	int num_fields = 0;
	int row_counter = 0;

	/* Connect to mysql database */
	con = mysql_init(NULL);
	if(con == NULL) {
		fprintf(stderr, "ERROR: Unable to initialize MySQL connection.\n");
		fprintf(stderr, "Error message: %s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
  
	if(mysql_real_connect(con, 
			      db_info->host, 
			      db_info->user, 
			      db_info->pass, 
			      NULL, 0, NULL, 0) == NULL) {
		fprintf(stderr, "%s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
  
	query = calloc(QUERY_STRING_SIZE, 1);

	// Select the database to use
	sprintf(query, "USE %s;", db_info->name);
	if(mysql_query(con, query)) {
		fprintf(stderr, "%s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
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
	if(NULL != con)
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

	/* Connect to mysql database */
	con = mysql_init(NULL);
	if(con == NULL) {
		fprintf(stderr, "ERROR: Unable to initialize MySQL connection.\n");
		fprintf(stderr, "Error message: %s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
  
	if(mysql_real_connect(con, 
			      db_info->host, 
			      db_info->user, 
			      db_info->pass, 
			      NULL, 0, NULL, 0) == NULL) {
		fprintf(stderr, "%s\n", mysql_error(con));
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
  
	// Select the database to use
	sprintf(query, "USE %s;", db_info->name);
	if(mysql_query(con, query)) {
		fprintf(stderr, "  %s\n", mysql_error(con));
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
