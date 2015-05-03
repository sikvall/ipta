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


/* Defines */

#define ERROR_FILE_OPEN -1
#define ACTION_IMPORT 1;

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

