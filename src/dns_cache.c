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


/*****************************************************************************
 * dns_cache_create_table creates a new table and populates it with
 * the necessary parths to be used later as a dns_cache for the ipta
 * system
 *****************************************************************************/
int dns_cache_create_table(struct ipta_db_info *db) {
}


/**********************************************************************
 * Delete a table pointed out by the struct and return success
 * RETVAL_OK if all went well or error code if there was a problem.
 **********************************************************************/
int dns_cache_delete_table(struct ipta_db_info *db) {
}


int dns_cache_clear_table(struct ipta_db_info *db) {
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
int dns_cache_drop_old_records(struct ipta_db_info *db) { 

}





int dns_cache_add(char *ip_address, char *hostname, 
		  struct ipta_db_info *db)
{
  char *query_string = NULL;
  MYSQL *con = NULL;
  MYSQL_RES *result = NULL;
  MYSQL_ROW row = 0;
  int retval = 0;

  /* Initialize databse object */
  con = mysql_init(NULL);
  if(con == NULL) {
    printf("! Unable to initialize MySQL connection.\n");
    printf("! Error message: %s\n", mysql_error(con));
    retval = 20;
    goto clean_exit;
  }
  
  /* Connect to database and check connection is sounds before proceeding. */
  if(mysql_real_connect(con, db->host, db->user, db->pass, NULL, 0, NULL, 0) == NULL) {
    fprintf(stderr, "! Error, unable to connect to database. Exiting.\n");
    fprintf(stderr, "  %s\n", mysql_error(con));
    mysql_close(con);
    retval = 20;
    goto clean_exit;
  }
  
  /* Select the database to use */
  sprintf(query_string, "USE %s;", db->name);
  if(mysql_query(con, query_string)) {
    printf("! Database %s not found, or not possible to connect. \n", db->name);
    printf("! %s\n", mysql_error(con));
    retval = 20;
    goto clean_exit;
  }
  
  
  /* Check if key exists */

  /* If exists update and reset ttl */

  /* If not exist then insert and set ttl */


 clean_exit:
  free(query_string);
  return retval;
}

int dns_cache_retrieve(char *ip_address, char *hostname, 
		       struct db_info *db_info)
{

}

