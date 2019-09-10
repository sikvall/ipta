/***********************************************************************
 * dns_cache-test.c
 *
 * Anders "Ichimusai" Sikvall
 * anders@sikvall.se
 *
 * Test framework for the dns cache system, not needed to compile the
 * tools, just the test for the dns cache framework.
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
#include <mysql.h>
#include "ipta.h"

#define TEST_I 1
#define TEST_II 1
#define TEST_III 1



int main(int argc, char *argv[]) 
{
	struct ipta_db_info db;
	int retval = RETVAL_OK;
	char hostname[256];
	
	printf("* Unit tests for the DNS cache subsystem of ipta.\n\n");
	
	// Populate the db struct with something that should work
	strcpy(db.host, "localhost");
	strcpy(db.user, "ipta");
	strcpy(db.pass, "ipta");
	strcpy(db.name, "ipta");
	strcpy(db.table, "dns");
	
	// Test I: Attemtp to create a table
	
#ifdef TEST_I
	
	fprintf(stderr, "* Test I: Create table.\n");
	retval = dns_cache_create_table(&db);
	if(retval != RETVAL_OK) 
		fprintf(stderr, "! Test error, unable to create the table.\n");
	else
		fprintf(stderr, "* Success!\n");

	
#endif
	
#ifdef TEST_II
	
	// Test II: Insert a new record in the database
	fprintf(stderr, "* Test II: Inserting a record in the database.\n");
	retval = dns_cache_add(&db, "10.0.0.1", "fake-hostname.tld");
	if(retval != RETVAL_OK)
		fprintf(stderr, "! Error, unable to perform test II with inserting new record.\n");
	else
		fprintf(stderr, "  Success.\n");
	
#endif
	
	// Test III: Select from the records the previously created one
	fprintf(stderr, "* Test III: Selecting the previously inserted record.\n");
	retval = dns_cache_get(&db, "10.0.0.1", hostname, "10");
	if(retval == RETVAL_OK) 
		fprintf(stderr, "  Found host in lookup: %s\n", hostname);
	else
		fprintf(stderr, "! Error: Unable to look up ip address.\n");
	
	// Test IV: Select a non-existent record
	fprintf(stderr, "* Test IV: Performing lookup on non-existent record\n");
	retval = dns_cache_get(&db, "10.42.0.1", hostname, "10");
	if(retval == RETVAL_OK)
		fprintf(stderr, "- Found host in lookup: %s\n", hostname);
	else
		fprintf(stderr, "  Unable to look up second ip address.\n"
			"  This is the expected behaviour at this stage.\n");
	

	fprintf(stderr, "* Test V: Prune all records.\n");
	retval = dns_cache_prune(&db, 0);
	
	return RETVAL_OK;
}
