/***********************************************************************
 * dns_cache-test.c
 *
 * Test framework for the dns cache system
 ***********************************************************************/

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


int main(int argc, char *argv[]) {
  struct ipta_db_info db;
  int retval = RETVAL_OK;

  /* Populate the db struct with something that should work */
  strcpy(db.host, "localhost");
  strcpy(db.user, "ipta");
  strcpy(db.pass, "ipta");
  strcpy(db.name, "ipta");
  strcpy(db.table, "dns");

  /* Attemtp to create a table */
  retval = dns_cache_create_table(&db);
  if(retval != RETVAL_OK) {
    fprintf(stderr, "! Test error, unable to create the table.\n");
    return RETVAL_ERROR;
  }
  


  return RETVAL_OK;
}
