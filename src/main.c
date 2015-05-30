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

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <my_global.h>
#include <mysql.h>
#include <linux/limits.h>
#include "ipta.h"
#include "cfg2.h"

#define DEBUG 1L


int main(int argc, char *argv[]) 
{
	struct ipta_flags *flags = NULL;
	struct ipta_db_info *db_info = NULL;
	struct ipta_db_info *dns_info = NULL;
	int i = 0;
	int retval = 0;
	char *import_fname = NULL;
	int clear_db = 0;
	int analyze_limit = 10;
	int known_flag, action_flag = 0;
	char password_input[1024];
	FILE *config_file = NULL;
	int print_usage_flag = 0;
	int create_table_flag = 0;
	char *follow_file = NULL;
	int follow_flag = 0;
	int create_db_flag = 0;
	int delete_table_flag = 0;
	int import_flag = 0;
	int analyze_flag = 0;
	int prune_dns_flag = 0;
	int list_tables_flg = 0;
	int dns_create_table_flag = 0;
	int dns_ttl = 24*14;
        //  int print_license_flag = 0;
	cfg_t *st; /* Configuration store */
	struct passwd *pw = NULL;
	char home[PATH_MAX];
	
	flags = calloc(sizeof(struct ipta_flags), 1);
	if(NULL == flags) {
		fprintf(stderr, "! Error, memory allocation failed.\n");
		exit(RETVAL_ERROR);
	}
	
	db_info = calloc(sizeof(struct ipta_db_info), 1);
	if(NULL == db_info) {
		fprintf(stderr, "! Error, memory allocation failed.\n");
		exit(RETVAL_ERROR);
	}
	
	/* Load default parameters */
	strcpy(db_info->host, DEFAULT_DB_HOSTNAME);
	strcpy(db_info->user, DEFAULT_DB_USERNAME);
	strcpy(db_info->pass, DEFAULT_DB_PASSWORD);
	strcpy(db_info->name, DEFAULT_DB_NAME);
	strcpy(db_info->table, DEFAULT_DB_TABLENAME);
	
	dns_info = calloc(sizeof(struct ipta_db_info), 1);
	if(NULL == db_info) {
		fprintf(stderr, "! Error, memory allocation failed.\n");
		exit(RETVAL_ERROR);
	}

	// Fixme - must be configurable later
	memcpy(dns_info, db_info, sizeof(struct ipta_db_info));
	strcpy(dns_info->table, "dns");

	/* Initialize config structure with no cache, we do not need it here
	   because there is not a lot of keywords to look at */
	
	st = calloc(sizeof(cfg_t), 1);
	retval = cfg_init(st, 0);
	if(retval) {
		fprintf(stderr,
			"! Error, unable to initialize configuration file struct.\n"
			"  This is an internal error and should be reported as a bug.\n"
			"  Information: cfg_init() returned %d.\n", retval);
		retval = RETVAL_ERROR;
		goto clean_exit;
	}
	
	/* Get user home dir and construct home path string */
	pw = getpwuid(getuid());
	retval = sprintf(home, "%s/.ipta", pw->pw_dir);
	retval = cfg_parse_file(st, home);
	if(!retval) {
		/* Look for the standard parameter names and move the value
		   to the internal hold as needed */
		i = 0;
		while(i < st->nkeys) {
			if(strlen(st->entry[i].value) >= IPTA_DB_INFO_STRLEN) {
				fprintf(stderr, "! Error in configuration file, key value too long.\n");
				fprintf(stderr, "  Key: %s\n", st->entry[i].key);
				retval = RETVAL_ERROR;
				goto clean_exit;
			}
			
			/* Check against the known keys, if match, copy value to hold */
			if(!strcmp("db_host", st->entry[i].key))
				strncpy(db_info->host,  st->entry[i].value, IPTA_DB_INFO_STRLEN);
			if(!strcmp("db_user", st->entry[i].key))
				strncpy(db_info->user,  st->entry[i].value, IPTA_DB_INFO_STRLEN);
			if(!strcmp("db_pass", st->entry[i].key))
				strncpy(db_info->pass,  st->entry[i].value, IPTA_DB_INFO_STRLEN);
			if(!strcmp("db_name", st->entry[i].key))
				strncpy(db_info->name,  st->entry[i].value, IPTA_DB_INFO_STRLEN);
			if(!strcmp("db_table", st->entry[i].key))
				strncpy(db_info->table, st->entry[i].value, IPTA_DB_INFO_STRLEN);
			if(!strcmp("dns_table", st->entry[i].key))
				strncpy(dns_info->table, st->entry[i].value, IPTA_DB_INFO_STRLEN);
			
			/* Analyzer limit is a little special and requires a range check */
			if(!strcmp("analyzer limit", st->entry[i].key)) {
				
				analyze_limit = strtol(st->entry[i].value, NULL, 10);
				
				if(analyze_limit < 1) {
					fprintf(stderr, "! Error, analyze limit must be > 0.");
					retval = RETVAL_ERROR;
					goto clean_exit;
				}
				
				if(analyze_limit > 1000) {
					fprintf(stderr, "! Error, analyze limit must be < 1000.\n");
					retval = RETVAL_ERROR;
					goto clean_exit;
				}
				
			} /* Is analyzer limit */ 
			
			i++; 
		} /* while keys left */
	} /* else */
	
	action_flag = FLAG_CLEAR;
	for(i=1; i < argc; i++) {
		known_flag = FLAG_CLEAR;
		
		if(!strcmp(argv[i], "--version") || 
		   !strcmp(argv[i], "-v")) {
			printf(IPTA_VERSION);
			retval = RETVAL_OK;
			continue;
		}
		
		if(!strcmp(argv[i], "--rdns") || 
		   !strcmp(argv[i], "-r")) {
			flags->rdns = FLAG_SET;
			known_flag = FLAG_SET;
		}
		
		if(!strcmp(argv[i], "--no-lo")) {
			flags->no_lo = FLAG_SET;
			known_flag = FLAG_SET;
		}
		
		if(!strcmp(argv[i], "--no-accept")) {
			flags->no_accept = FLAG_SET;
			known_flag = FLAG_SET;
		}
		
		if(!strcmp(argv[i], "--license")) {
			print_license();
			goto clean_exit;
		}
		
		if(!strcmp(argv[i], "--usage")) {
			known_flag = FLAG_SET;
			print_usage_flag = FLAG_SET;
			action_flag = FLAG_SET;
		}
		
		
		if(!strcmp(argv[i], "--create-db") || 
		   !strcmp(argv[i], "-cd")) {
			known_flag = FLAG_SET;
			action_flag = FLAG_SET;
			create_db_flag = FLAG_SET;
		}
		
		if(!strcmp(argv[i], "--follow") || 
		   !strcmp(argv[i], "-f")) {
			known_flag = FLAG_SET;
			action_flag = FLAG_SET;
			follow_flag = FLAG_SET;
			
			if ( argc < (i+2) ) {
				fprintf(stderr, "! Error: Follow mode needs a file name!\n");
				retval = RETVAL_ERROR;
				goto clean_exit;
			}
			
			follow_file = argv[i+1];
			i++;
			continue;
		}
		
		if(!strcmp(argv[i], "-l") || 
		   !strcmp(argv[i], "--limit")) {
			known_flag = FLAG_SET;
			if(argc < (i+2)) {
				fprintf(stderr, "? You need to supply an argument for the limit.\n");
				retval = RETVAL_ERROR;
				goto clean_exit;
			}
			analyze_limit = atoi(argv[i+1]); 
			i++;
			if(analyze_limit < 1) {
				fprintf(stderr, "! Invalid analyze limit %d must be at least 1.\n", analyze_limit);
				retval = RETVAL_ERROR;
				goto clean_exit;
			}
			if(analyze_limit > ANALYZE_LIMIT_MAX) {
				printf("! To high limit %d, should be <= %d.\n", analyze_limit, ANALYZE_LIMIT_MAX);
				return RETVAL_ERROR;
			}
			continue;
		}
		
		if(!strcmp(argv[i], "--db-name") || 
		   !strcmp(argv[i], "-d")) {
			known_flag = FLAG_SET;
			if(argc < (i+2)) {
				fprintf(stderr, "! You must supply a name with argument %s\n", argv[i]);
				retval = RETVAL_ERROR;
				goto clean_exit;
			}
			i++;
			strncpy(db_info->name, argv[i], IPTA_DB_INFO_STRLEN);
			continue;
		}
		
		if(!strcmp(argv[i], "--db-user") || !strcmp(argv[i], "-u")) {
			known_flag = FLAG_SET;
			if(argc < (i+2)) {
				fprintf(stderr, "! You must supply a name with argument %s\n", argv[i]);
				retval = RETVAL_ERROR;
				goto clean_exit;
			}
			i++;
			strncpy(db_info->user, argv[i], IPTA_DB_INFO_STRLEN);
			continue;
		}
		
		if(!strcmp(argv[i], "--db-table") || 
		   !strcmp(argv[i], "-t")) {
			known_flag = FLAG_SET;
			if(argc < (i+2)) {
				fprintf(stderr, "! You must supply a name with argument %s\n", argv[i]);
				retval = RETVAL_ERROR;
				goto clean_exit;
			}
			i++;
			strncpy(db_info->table, argv[i], IPTA_DB_INFO_STRLEN);
			continue;
		}
		
		if(!strcmp(argv[i], "--db-pass-i") || 
		   !strcmp(argv[i], "-pi")) {
			known_flag = FLAG_SET;
			printf("? Enter your password: ");
			if(1 != scanf("%s", password_input)) {
				fprintf(stderr, "! Error, password not properly given.\n");
				return RETVAL_ERROR;
			}
			strncpy(db_info->pass, password_input, IPTA_DB_INFO_STRLEN);
			continue;
		}
		
		if(!strcmp(argv[i], "--db-name") || 
		   !strcmp(argv[i], "-d")) {
			known_flag = FLAG_SET;
			if(argc < (i+2)) {
				fprintf(stderr, "? If you want to use use %s to select database, I need a name.\n", argv[i]);
				retval = RETVAL_ERROR;
				goto clean_exit;
			}
			strncpy(db_info->name, argv[i+1], IPTA_DB_INFO_STRLEN); 
			i++;
			continue;
		}
		
		if(!strcmp(argv[i], "--db-host") || 
		   !strcmp(argv[i], "-h")) {
			known_flag = FLAG_SET;
			if(argc < (i+2)) {
				fprintf(stderr, "? If you want to use use %s to select database host, I need a name.\n", argv[i]);
				retval = RETVAL_ERROR;
				goto clean_exit;
			}
			strncpy(db_info->host, argv[i+1], IPTA_DB_INFO_STRLEN); 
			i++;
			continue;
		}
		
		if(!strcmp(argv[i], "--db-pass") || 
		   !strcmp(argv[i], "-p")) {
			fprintf(stderr, 
				"! Warning; for security reasons option is discouraged and instead\n" \
				"  --db-pass-i or -pi is recommended.\n");
			known_flag = FLAG_SET;
			if(argc < (i+2)) {
				fprintf(stderr, 
					"? If you want to use use %s to set password, I need a password.\n", argv[i]);
				retval = RETVAL_ERROR;
				goto clean_exit;
			}
			strncpy(db_info->pass, argv[i+1], IPTA_DB_INFO_STRLEN); 
			i++;
			continue;
		}
		

		if(!strcmp(argv[i], "--dns-ttl")) {
			if(argc < (i+2)) {
				fprintf(stderr, "? Missing argument for --dns-ttl\n");
				retval = RETVAL_ERROR;
				goto clean_exit;
			}
			dns_ttl = atoi(argv[i+1]);
			if(!dns_ttl) {
				fprintf(stderr, "! Error, dns_ttl should not be 0.\n");
				retval = RETVAL_ERROR;
				goto clean_exit;
			}
			i++; /* Increase to swallow argument */
			continue;
		}

		if(!strcmp(argv[i], "--dns-create-table")) {
			known_flag = FLAG_SET;
			action_flag = FLAG_SET;
			dns_create_table_flag = FLAG_SET;
		}
		
		/* Table operations defined here as flags are processed */
		if(!strcmp(argv[i], "-lt") || 
		   !strcmp(argv[i], "--list-tables")) {
			known_flag = FLAG_SET;
			action_flag = FLAG_SET;
			list_tables_flg = FLAG_SET;
			continue;
		}
		
		if(!strcmp(argv[i], "-dt") || 
		   !strcmp(argv[i], "--delete-table")) {
			known_flag = FLAG_SET;
			action_flag = FLAG_SET;
			delete_table_flag = FLAG_SET;
			continue;
		}
		
		if(!strcmp(argv[i], "--dns-prune") ||
		   !strcmp(argv[i], "-dp")) {
			known_flag = FLAG_SET;
			prune_dns_flag = FLAG_SET;
			action_flag = FLAG_SET;
		}

		if(!strcmp(argv[i], "--create-table") || 
		   !strcmp(argv[i], "-ct")) {
			known_flag = FLAG_SET;
			action_flag = FLAG_SET;
			create_table_flag = FLAG_SET;
			continue;
		}
		
		if(!strcmp(argv[i], "-c") || 
		   !strcmp(argv[i], "--clear")) {
			known_flag = FLAG_SET;
			action_flag = FLAG_SET;
			clear_db = FLAG_SET;
			continue;
		}
		
		if(!strcmp(argv[i], "-i") || 
		   !strcmp(argv[i], "--import")) {
			action_flag = FLAG_SET;
			known_flag = FLAG_SET;
			if(argc < (i+2)) {
				fprintf(stderr, 
					"? To import syslog you need to specify a filename after '%s'.\n", argv[i]);
				retval = RETVAL_WARN;
				goto clean_exit;
			}
			import_fname = argv[i+1]; i++;
			import_flag = FLAG_SET;
			continue;
		}
		
		if(!strcmp(argv[i], "--analyze") || 
		   !strcmp(argv[i], "-a")) {
			analyze_flag = FLAG_SET;
			action_flag = FLAG_SET;
			known_flag = FLAG_SET;
		}

		
		/* Sanity check for arguments we don't handle */
		if(!known_flag) {
			fprintf(stderr, "Unknown option or argument %s found.\n", argv[i]);
			retval = RETVAL_WARN; 
			goto clean_exit;
		}
	}


	/***********************************************************************
	 * Process the various flags in the correct order no matter
	 * how they were inserted on the command line.
	 ***********************************************************************/
	
	/* Process the actual modes to do something here */
	if(!action_flag) {
		fprintf(stderr, "- No action, exiting.\n");
		retval = RETVAL_WARN;
		goto clean_exit;
	}
	
	/* This must be the first action as it may break all the
	 * others except for the dns settings */

	if(prune_dns_flag) {
		retval = dns_cache_prune(dns_info, dns_ttl);
	}

	if(dns_create_table_flag) {
		retval = dns_cache_create_table(dns_info);
		if(retval) {
			fprintf(stderr, "! Error in creating DNS table.\n");
			goto clean_exit;
		}
	}

	if(follow_flag) {
		retval = follow(follow_file, flags, dns_info);
		goto clean_exit;
	}
	
	/* Print the usage of ipta */
	if(print_usage_flag) {
		print_usage();
	}
	
	/* Setup the default db setting and stor in .ipta */
	if(create_db_flag) {
		retval = create_db(db_info);
		if(!retval) {
			fprintf(stderr, 
				"! Error, create db failed, exiting. You need to give MySQL root privileges\n"
				"  for this to work as the database must be created and a grand given to ipta.\n");
			goto clean_exit;
		}
	}
	
	/* Show the different tables used in the database */
	if(list_tables_flg) {
		retval = list_tables(db_info);
		if(retval)
			goto clean_exit;
		/* If retval is FALSE then there is no problem and we can continue
		   to process other things. */
	}
	
	if(delete_table_flag) {
		fprintf(stderr, "! Delete table is a stub and not yet implemented.\n");
	}
	
	/* Create the prime table needed to import the syslog entries */
	if(create_table_flag) {
		retval = create_table(db_info);
		if(retval)
			goto clean_exit;
	}

	/* Clear all database entries */
	if(clear_db) {
		retval = clear_database(db_info);
		if(retval != 0) {
			fprintf(stderr, "! Error, clearing database, exiting.\n");
			goto clean_exit;
		} 
	}
	
	
	if(import_flag) {
		/* import from syslog */
		retval = import_syslog(db_info, import_fname);
		if(retval != 0) {
			fprintf(stderr, "! Error importing. Sorry.\n");
			goto clean_exit;
		} 
	}

	/* Run the automatic analyzer module */
	if(analyze_flag) {
		retval = analyze(db_info, flags, analyze_limit, dns_info);
		if(retval) {
			fprintf(stderr, "! Error in analyzer. Sorry.\n");
			retval = RETVAL_ERROR;
			goto clean_exit;
		}
	}

clean_exit:

	if(config_file)
		fclose(config_file);
	free(flags);
	free(db_info);
	free(dns_info);
	cfg_free(st);

	return retval;
}
