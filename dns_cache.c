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

int dns_cache_add(char *ip_address, char *hostname, 
		  struct ipta_db_info *db)
{
  char *query_string = NULL;
  MYSQL *con = NULL;
  MYSQL_RES *result = NULL;
  MYSQL_ROW row = 0;
  int retval = 0;

  con = mysql_init(NULL);
  if(con == NULL) {
    printf("! Unable to initialize MySQL connection.\n");
    printf("! Error message: %s\n", mysql_error(con));
    retval = 20;
    goto clean_exit;
  }
  
  if(mysql_real_connect(con, 
			db->host, 
			db->user, 
			db->pass, 
			NULL, 0, NULL, 0) == NULL) {
    printf("! %s\n", mysql_error(con));
    mysql_close(con);
    retval = 20;
    goto clean_exit;
  }
  
  // Select the database to use
  sprintf(query_string, "USE %s;", db->name);
  if(mysql_query(con, query_string)) {
    printf("! Database %s not found, or not possible to connect. \n", db->name);
    printf("! %s\n", mysql_error(con));
    retval = 20;
    goto clean_exit;
  }
  
  /* insert a test record */

 clean_exit:
  
  return retval;
  free(query_string);

}

int dns_cache_retrieve(char *ip_address, char *hostname, 
		       struct db_info *db_info)
{

}


void main(int argc, char *argv[])
{
  struct ipta_db_info *db_info = NULL;
  char *query_string = NULL;

  if(argc != 3) {
    printf("Not the right number of arguments, exiting.\n");
    exit(10);
  }

  /* allocate memory for db structure */
  if(! calloc(db_info, sizeof(struct ipta_db_info)))
    exit(20);

  /* set it up for a testrun */
  strcpy(db_info->host,  "localhost");
  strcpy(db_info->user,  "ipta");
  strcpy(db_info->pass,  "ipta");
  strcpy(db_info->name,  "ipta");
  strcpy(db_info->table, "logs");
  
  

 clean_exit:

  free(db_info);
  free(query_string);

  return 0;
}
