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

//#define TEST_I 1
#define TEST_II 1
#define TEST_III 1



int main(int argc, char *argv[]) {
  struct ipta_db_info db;
  int retval = RETVAL_OK;
  char hostname[256];


  /* Populate the db struct with something that should work */
  strcpy(db.host, "localhost");
  strcpy(db.user, "ipta");
  strcpy(db.pass, "ipta");
  strcpy(db.name, "ipta");
  strcpy(db.table, "dns");

  /* Test I: Attemtp to create a table */

#ifdef TEST_I

  retval = dns_cache_create_table(&db);
  if(retval != RETVAL_OK) {
    fprintf(stderr, "! Test error, unable to create the table.\n");
    return RETVAL_ERROR;
  }
  fprintf(stderr, "+ Test I: Creating the table passed.\n");

#endif
  
#ifdef TEST_II

  /* Test II: Insert a new record in the database */
  fprintf(stderr, "- Test II: Inserting a record in the database.\n");
  retval = dns_cache_add(&db, "10.0.0.1", "fake-hostname.tld");
  if(retval != RETVAL_OK) {
    fprintf(stderr, "! Error, unable to perform test II with inserting new record");
    return(RETVAL_ERROR);
  }
  perror("+ Test II Completed successfully!\n");

#endif

  /* Test III: Select from the records the previously created one */

  retval = dns_cache_get(&db, "10.0.0.1", &hostname, "10");
  if(retval == RETVAL_OK) {
    fprintf(stderr, "- Found host in lookup: %s\n", hostname);
  } else {
    fprintf(stderr, "! Error: Unable to look up ip address.\n");
    return RETVAL_ERROR;
  }
  

  /* Test IV: Select a non-existent record */
  retval = dns_cache_get(&db, "10.42.0.1", &hostname, "10");
  if(retval == RETVAL_OK) {
    fprintf(stderr, "- Found host in lookup: %s\n", hostname);
  } else {
    fprintf(stderr, "! Error: Unable to look up ip address.\n");
    return RETVAL_ERROR;
  }


  return RETVAL_OK;
}
