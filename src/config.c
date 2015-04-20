/**********************************************************************
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


/**********************************************************************
 * Create configuration file with default values
 *
 * Parameters: none
 *
 * Returns: RETVAL_OK on success and RETVAL_ERROR on error
 **********************************************************************/
int create_config(void) {
  struct ipta_config *config;

  config = calloc(sizeof(struct ipta_config), 1);
  if(!config) {
    fprintf(stderr, "! Unable to allocate memory for config.\n");
    return RETVAL_ERROR;
  }

  /* Populate the struct with standard values */
  strncpy(config->db_host,  DEFAULT_DB_HOSTNAME,  IPTA_DB_INFO_STRLEN);
  strncpy(config->db_user,  DEFAULT_DB_USERNAME,  IPTA_DB_INFO_STRLEN);
  strncpy(config->db_pass,  DEFAULT_DB_PASSWORD,  IPTA_DB_INFO_STRLEN);
  strncpy(config->db_name,  DEFAULT_DB_NAME,      IPTA_DB_INFO_STRLEN);
  strncpy(config->db_table, DEFAULT_DB_TABLENAME, IPTA_DB_INFO_STRLEN);

  fprintf(stderr, "! This is a stub function create_config().\n");
  return RETVAL_ERROR;
    
}

int read_config(struct ipta_config *config, char *filename) {
  FILE *fp = NULL;
  int num = 0;
  

  fopen(filename, "r");
  if(!fp) {
    fprintf(stderr, "! Unable to open file '%s'.\n", filename);
    return RETVAL_ERROR;
  }

  fprintf(stderr, "! config file reading a stub so far.\n");
  return RETVAL_ERROR;

  // File is open, process the keywords

}

int update_config(struct ipta_config *config) {
  fprintf(stderr, "! The update_confi() function is a stub.\n");
  return RETVAL_ERROR;
}

