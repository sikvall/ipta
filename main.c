/**********************************************************************
 * main.c
 *
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
 * software. For the terms and conditions for this software you
 * should refer to the "LICENCE" file in the source directory or run
 * a compiled binary with the "--licence" option which will display
 * the licence.
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

#define IPTA_VERSION "ipta version 1.0.0\n"

int main(int argc, char *argv[]) 
{
  struct ipta_flags *flags = NULL;
  struct ipta_db_info *db_info = NULL;
  int i = 0;
  int retval = 0;
  char *import_fname = NULL;
  int clear_db = 0;
  int analyze_limit = 10;
  int known_flag, action_flag = 0;
  char input_db_pass[512];
  char hostname[256];
  char password_input[1024];
  FILE *config_file = NULL;
  int print_usage_flag = 0;
  int create_table_flag = 0;
  char *follow_file = NULL;
  int follow_flag = 0;
  int analyze_limits = 20;
  int save_db_flag = 0;
  int delete_table_flag = 0;
  int import_flag = 0;
  int analyze_flag = 0;
  int list_tables_flg = 0;
  int print_license_flag = 0;
  
  flags = calloc(sizeof(struct ipta_flags), 1);
  if(NULL == flags) {
    fprintf(stderr, "! Error, memory allocation failed.\n");
    exit(20);
  }
  
  db_info = calloc(sizeof(struct ipta_db_info), 1);
  if(NULL == db_info) {
    fprintf(stderr, "! Error, memory allocation failed.\n");
    exit(20);
  }
  
  // Load default parameters
  strcpy(db_info->host, "localhost");
  strcpy(db_info->user, "ipta");
  strcpy(db_info->pass, "ipta");
  strcpy(db_info->name, "ipta");
  strcpy(db_info->table, "logs");

  // Check if there is a config file, if so we should parse that for our
  // data before we override that with something on the command line

  config_file = fopen("~/.ipta", "r");
  if(NULL != config_file) {
    fprintf(stderr, "o Opening config file is a stub right now.\n");
    // File exists so we look for keywords in the file to match to our
    // parameters but any missing keyword will be default
    
    // TO BE DONE
  }

  action_flag = 0;
  for(i=1; i < argc; i++) {
    known_flag = 0;

    //    printf("* Checking argument %s\n", argv[i]);

    if(!strcmp(argv[i], "--version") || !strcmp(argv[i], "-v")) {
      printf(IPTA_VERSION);
      goto clean_exit;
    }

    if(!strcmp(argv[i], "--rdns") || !strcmp(argv[i], "-r")) {
      flags->rdns = 1;
      known_flag = 1;
    }

    if(!strcmp(argv[i], "--no-lo")) {
      flags->no_lo = 1;
      known_flag = 1;
    }

    if(!strcmp(argv[i], "--no-accept")) {
      flags->no_accept = 1;
      known_flag = 1;
    }

    if(!strcmp(argv[i], "--license")) {
      known_flag = 1;
      print_license_flag = 1;
      action_flag = 1;
    }

    if(!strcmp(argv[i], "--usage")) {
      known_flag = 1;
      print_usage_flag = 1;
      action_flag = 1;
    }

    if(!strcmp(argv[i], "--setup-db") || !strcmp(argv[i], "-s")) {
      known_flag = 1;
      action_flag = 1;
      save_db_flag = 1;
    }

    if(!strcmp(argv[i], "--follow") || !strcmp(argv[i], "-f")) {
      known_flag = 1;
      action_flag = 1;
      follow_flag = 1;

      if ( argc < (i+2) ) {
	fprintf(stderr, "! Error: Follow needs a file name!\n");
	retval = 20;
	goto clean_exit;
      }

      follow_file = argv[i+1];
      i++;
      continue;
    }

    if(!strcmp(argv[i], "-l") || !strcmp(argv[i], "--limit")) {
      known_flag = 1;
      if(argc < (i+2)) {
	fprintf(stderr, "? You need to supply an argument for the limit.\n");
	retval = 20;
	goto clean_exit;
      }
      analyze_limit = atoi(argv[i+1]); i++;
      if(analyze_limit < 1) {
	fprintf(stderr, "! Invalid analyze limit %d must be at least 1.\n", analyze_limit);
	retval = 20;
	goto clean_exit;
      }
      if(analyze_limit > 1000) {
	printf("! To high limit %d, should be < 1000.\n", analyze_limit);
	return 20;
      }
      continue;
    }

    if(!strcmp(argv[i], "--db-name") || !strcmp(argv[i], "-d")) {
      known_flag = 1;
      if(argc < (i+2)) {
	fprintf(stderr, "! You must supply a name with argument %s\n", argv[i]);
	retval = 20;
	goto clean_exit;
      }
      i++;
      strncpy(db_info->name, argv[i], IPTA_DB_INFO_STRLEN);
      continue;
    }

    if(!strcmp(argv[i], "--db-user") || !strcmp(argv[i], "-u")) {
      known_flag = 1;
      if(argc < (i+2)) {
	fprintf(stderr, "! You must supply a name with argument %s\n", argv[i]);
	retval = 20;
	goto clean_exit;
      }
      i++;
      strncpy(db_info->user, argv[i], IPTA_DB_INFO_STRLEN);
      continue;
    }
	 
    if(!strcmp(argv[i], "--db-table") || !strcmp(argv[i], "-t")) {
      known_flag = 1;
      if(argc < (i+2)) {
	fprintf(stderr, "! You must supply a name with argument %s\n", argv[i]);
	retval = 20;
	goto clean_exit;
      }
      i++;
      strncpy(db_info->table, argv[i], IPTA_DB_INFO_STRLEN);
      continue;
    }

    if(!strcmp(argv[i], "--db-pass-i") || !strcmp(argv[i], "-pi")) {
      known_flag = 1;
      printf("? Enter your password: ");
      scanf("%s", password_input);
      strncpy(db_info->pass, password_input, IPTA_DB_INFO_STRLEN);
      continue;
    }
	 
    if(!strcmp(argv[i], "--db-name") || !strcmp(argv[i], "-d")) {
      known_flag = 1;
      if(argc < (i+2)) {
	fprintf(stderr, "? If you want to use use %s to select database, I need a name.\n", argv[i]);
	retval = 20;
	goto clean_exit;
      }
      strncpy(db_info->name, argv[i+1], IPTA_DB_INFO_STRLEN); 
      i++;
      continue;
    }

    if(!strcmp(argv[i], "--db-host") || !strcmp(argv[i], "-h")) {
      known_flag = 1;
      if(argc < (i+2)) {
	fprintf(stderr, "? If you want to use use %s to select database host, I need a name.\n", argv[i]);
	retval = 20;
	goto clean_exit;
      }
      strncpy(db_info->host, argv[i+1], IPTA_DB_INFO_STRLEN); 
      i++;
      continue;
    }
    
    if(!strcmp(argv[i], "--db-pass") || !strcmp(argv[i], "-p")) {
      fprintf(stderr, 
	      "! Warning for security reasons option is discouraged and instead\n" \
	      "  --db-pass-i or -pi is recommended.\n");
      known_flag = 1;
      if(argc < (i+2)) {
	fprintf(stderr, 
		"? If you want to use use %s to set password, I need a password.\n", argv[i]);
	retval = 20;
	goto clean_exit;
      }
      strncpy(db_info->pass, argv[i+1], IPTA_DB_INFO_STRLEN); i++;
      continue;
    }


    /* Table operations defined here as flags are processed */

    if(!strcmp(argv[i], "-lt") || !strcmp(argv[i], "--list-tables")) {
      known_flag = 1;
      action_flag = 1;
      list_tables_flg = 1;
      continue;
    }

    if(!strcmp(argv[i], "-dt") || !strcmp(argv[i], "--delete-table")) {
      known_flag = 1;
      action_flag = 1;
      delete_table_flag = 1;
      continue;
    }

    if(!strcmp(argv[i], "--create-table") || !strcmp(argv[i], "-ct")) {
      known_flag = 1;
      action_flag = 1;
      create_table_flag = 1;
      continue;
    }

    if(!strcmp(argv[i], "-c") || !strcmp(argv[i], "--clear-db")) {
      known_flag = 1;
      action_flag = 1;
      clear_db = 1;
      continue;
    }

    if(!strcmp(argv[i], "-i") || !strcmp(argv[i], "--import")) {
      printf("Import\n");
      action_flag = 1;
      known_flag = 1;
      if(argc < (i+2)) {
	fprintf(stderr, "? To import syslog you should specify a filename after %s.\n", argv[i]);
	retval = 10;
	goto clean_exit;
      }
      import_fname = argv[i+1]; i++;
      import_flag = 1;
      continue;
    }
    
    if(!strcmp(argv[i], "-a") || !strcmp(argv[i], "--analyze")) {
      analyze_flag = 1;
      action_flag = 1;
      known_flag = 1;
    }
    
    // Sanity check for arguments we don't handle
    if(!known_flag) {
      fprintf(stderr, "Unknown option or argument %s found.\n", argv[i]);
      retval = 10; 
      goto clean_exit;
    }
  }

  // Process the actual modes to do something here
  if(!action_flag) {
    fprintf(stderr, "No action. Exit.\n");
    retval = 10;
    goto clean_exit;
  }

  /* This must be the first action as it may break all the others */
  if(follow_flag) {
    retval = follow(follow_file, flags);
    goto clean_exit;
  }

  if(print_usage_flag) {
    print_usage();
  }

  /* Setup the default db setting and stor in .ipta */
  if(save_db_flag) {
    retval = save_db(db_info);
    if(!retval) {
      fprintf(stderr, "! Error, save db failed, exiting.\n");
      goto clean_exit;
    }
  }

  if(list_tables_flg) {
    retval = list_tables(db_info);
    if(retval)
      goto clean_exit;
    // If retval is FALSE then there is no problem and we can
    // continue to process other things.
  }

  if(create_table_flag) {
    retval = create_table(db_info);
    if(retval)
      goto clean_exit;
  }

  if(clear_db) {
    // Clear the database
    retval = clear_database(db_info);
    if(retval != 0) {
      fprintf(stderr, "! Error, clearing database, exiting.\n");
      goto clean_exit;
    } else {
      fprintf(stderr, "* Database cleared.\n");
    }
  }
  
  if(import_flag) {
    retval = import_syslog(db_info, import_fname);
    if(retval != 0) {
      fprintf(stderr, "! Error importing. Sorry.\n");
      goto clean_exit;
    } else {
      fprintf(stderr, "* Done importing.\n");
    }
  }
  
  if(analyze_flag) {
    retval = analyze(db_info, flags, analyze_limits);
    if(retval) {
      fprintf(stderr, "! Error in analyzer. Sorry.\n");
      retval = 20;
      goto clean_exit;
    }
  }
  
  if(print_license_flag) {
    print_license();
  }
  
  
 clean_exit:
  if(config_file)
    fclose(config_file);
  free(flags);
  free(db_info);
  return retval;
}
