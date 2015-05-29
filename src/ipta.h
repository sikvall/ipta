/**********************************************************************
 * ipta.h
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


/* Overall generic defines */
#define IPTA_VERSION "ipta version Release-V0.3\n"

/* Specific error defines */
#define ERROR_FILE_OPEN -1

/* Ipta defines */
#define ACTION_IMPORT 1;
#define IPTA_LINE_PREFIX "IPT: "
#define QUERY_STRING_SIZE 2000
#define QUERY_ROW_COUNT 100

/* Database defaults */
#define DEFAULT_DB_HOSTNAME "localhost"
#define DEFAULT_DB_USERNAME "ipta"
#define DEFAULT_DB_PASSWORD "ipta"
#define DEFAULT_DB_NAME "ipta"
#define DEFAULT_DB_TABLENAME "logs"

/* Flags */
#define FLAG_SET 1
#define FLAG_CLEAR 0

/* Defined return values from functions and from the application
   itself */
#define RETVAL_OK 0
#define RETVAL_WARN 10
#define RETVAL_ERROR 20
#define RETVAL_NONAME 1

/* Specific defines */
#define HOSTNAME_MAX_LEN 256
#define ANALYZE_LIMIT_MAX 1000
#define CONFIG_FILE_PATH "~/.ipta/config"

struct ipta_flags {
	int no_lo;
	int rdns;
	int no_accept;
};

#define IPTA_DB_INFO_STRLEN 256
struct ipta_db_info {
	char host[IPTA_DB_INFO_STRLEN];
	char user[IPTA_DB_INFO_STRLEN];
	char pass[IPTA_DB_INFO_STRLEN];
	char name[IPTA_DB_INFO_STRLEN];
	char table[IPTA_DB_INFO_STRLEN];
};

struct ipta_config {
	char db_host[IPTA_DB_INFO_STRLEN];
	char db_user[IPTA_DB_INFO_STRLEN];
	char db_pass[IPTA_DB_INFO_STRLEN];
	char db_name[IPTA_DB_INFO_STRLEN];
	char db_table[IPTA_DB_INFO_STRLEN];
	struct ipta_flags *flags;
};


/* Function declarations */
int analyze(struct ipta_db_info *db, struct ipta_flags *flags, int analyze_limit);
int open_db(struct ipta_db_info *db);
int create_config(void);
int restore_db(struct ipta_db_info *db);
int save_db(struct ipta_db_info *db);
int create_database(struct ipta_db_info *db);
int create_table(struct ipta_db_info *db);
int delete_table(struct ipta_db_info *db);
int list_tables(struct ipta_db_info *db);
int clear_database(struct ipta_db_info *db);
int follow(char *filename, struct ipta_flags *flags);
int get_host_by_addr(char *ip_address, char *hostname, int maxlen);
int import_syslog(struct ipta_db_info *db, char *filename);
void print_license(void);
void print_usage(void);

/* dns cache prototypes */
int dns_cache_create_table(struct ipta_db_info *db);
int dns_cache_add(struct ipta_db_info *db, char *ip_address, char *hostname);
int dns_cache_get(struct ipta_db_info *db, char *ip_address, char *hostname, char *ttl);
int dns_cache_delete_table(struct ipta_db_info *db);
int dns_cache_clear_table(struct ipta_db_info *db);
int dns_cache_prune(struct ipta_db_info *db); /* This should change to include ttl */
